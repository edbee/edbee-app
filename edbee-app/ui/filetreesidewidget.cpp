/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QAction>
#include <QComboBox>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>
#include <QTreeView>
#include <QVBoxLayout>


#include "application.h"
#include "filetreesidewidget.h"
#include "models/workspace.h"
#include "QtAwesome.h"
#include "util/fileutil.h"

#include "edbee/debug.h"


/// Constructs the file side widget
/// @param parent the parent widget
FileTreeSideWidget::FileTreeSideWidget(QWidget* parent)
    : QWidget(parent)
    , fileTreeModel_(0)
    , fileTreeRef_(0)
    , rootPathList_(0)
    , workspaceRef_(0)
{
    constructUI();
    connectSignals();
    rootPathList_ = new QStringListModel(QStringList("/"));
}


/// The file tree side widget destructor
FileTreeSideWidget::~FileTreeSideWidget()
{
    delete rootPathList_;
    delete fileTreeModel_;
}


/// sets the project
/// @param project the project that's set
void FileTreeSideWidget::setWorkspace( Workspace* project )
{
    workspaceRef_ = project;
    pathComboRef_->setModel( rootPathList_ );
}


/// Returns the project
Workspace*FileTreeSideWidget::workspace() const
{
    return workspaceRef_;
}


/// Serializes the state of the file tree widget to a variant map
QVariantMap FileTreeSideWidget::serialize()
{
    QVariantMap result;
    QVariantList paths;
    for( int i=1, cnt=pathComboRef_->count(); i<cnt; ++i ) {
        paths.push_back( pathComboRef_->itemText(i));
    }
    result.insert("paths",paths);
    result.insert("active-path-index", pathComboRef_->currentIndex() );
    return result;
}


/// deserializes the given variant map to the state of this tree widget
void FileTreeSideWidget::deserialize(const QVariantMap& map)
{
    // remove all rootpaths currently available
    clearAllRootPaths();

    // add all paths to the root combobox list
    QVariantList paths = map.value("paths").toList();
    for( int i=0,cnt=paths.size(); i<cnt; ++i ) {
        QString path = paths.at(i).toString();
        if( !path.isEmpty() ) {
            pathComboRef_->addItem(path);
        }
    }

    // change the curent index
    int idx = map.value("active-path-index").toInt();
    idx = qBound(0,idx,pathComboRef_->count()-1);
    pathComboRef_->setCurrentIndex(idx);
}


/// The file tree is double clicked
void FileTreeSideWidget::openFileItem( const QModelIndex& index)
{
    // when clicking a file
    QFileInfo fileInfo = fileTreeModel_->fileInfo(index);
    if( !fileInfo.isFile() ) { return; }

    // open the file and add a tab
    emit fileDoubleClicked( fileInfo.absoluteFilePath() );
    //openFile( fileInfo.absoluteFilePath() );

}


/// Opens the  file tree context menu at the given point
/// @param point the point to open the context menu
void FileTreeSideWidget::fileTreeContextMenu(const QPoint& point)
{
    // for most widgets
    QPoint globalPos = fileTreeRef_->mapToGlobal(point);

    // get the file info
    QModelIndex index = fileTreeRef_->indexAt(point);

     // for QAbstractScrollArea and derived classes you would use:
     // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    // build the context menu
    QMenu menu;

    // get the file info
    QFileInfo fileInfo;
    if( index.isValid() ) {
        fileInfo = fileTreeModel_->fileInfo(index);
    }


    // GOTO Menu building
    QMenu* gotoMenu = menu.addMenu("Goto");
    if( index.isValid() ) {

        // add the clicked file to the goto menu when the file is a dir
        if( fileInfo.isDir() ) {
            QAction* changeRootAction = new QAction(tr("%1").arg(fileInfo.fileName()), &menu );
            changeRootAction->setData( fileInfo.absoluteFilePath() );
            connect(changeRootAction, SIGNAL(triggered()), this, SLOT(setRootPathByAction()) );
            gotoMenu->addAction(changeRootAction);
        }

        // add a rename action
        QAction* renameAction = new QAction(tr("Rename.."),&menu);
        renameAction->setData( fileInfo.absoluteFilePath() );
        connect( renameAction, SIGNAL(triggered()), this, SLOT(startRenameItemByAction()) );
        menu.addAction( renameAction );

        // add a delete action
        QAction* deleteAction = new QAction(tr("Delete..."),&menu);
        deleteAction->setData( fileInfo.absoluteFilePath() );
        connect( deleteAction, SIGNAL(triggered()), this, SLOT(deleteItemByAction()) );
        menu.addAction( deleteAction );

        if( fileInfo.isFile() ) {
            // add a duplicate action
            QAction* duplicateAction = new QAction(tr("Duplicate"),&menu);
            duplicateAction->setData( fileInfo.absoluteFilePath() );
            connect( duplicateAction, SIGNAL(triggered()), this, SLOT(duplicateFileAndRenameByAction()) );
            menu.addAction( duplicateAction );
        }

        // when clicking on a directory add a create new file/folder options
        if( fileInfo.isDir() ) {
            // create new file aciton
            QAction* newFileAction = new QAction( tr("New File"), &menu);
            newFileAction ->setData( fileInfo.absoluteFilePath() );
            connect( newFileAction , SIGNAL(triggered()), this, SLOT(createNewFileAndRenameByAction()) );
            menu.addAction( newFileAction );

            // create new folder action
            QAction* newFolderAction = new QAction( tr("New Folder"), &menu);
            newFolderAction ->setData( fileInfo.absoluteFilePath() );
            connect( newFolderAction , SIGNAL(triggered()), this, SLOT(createNewFolderAndRenameByAction()) );
            menu.addAction( newFolderAction );
        }

        // add a reveal action.
        // Todo, 'dry' this, same construct is used in MainWindow class!
        QString revealText =  tr("Reveal in Filesystem");
#ifdef Q_OS_MAC
        revealText =  tr("Reveal in Finder");
#elif defined(Q_OS_WIN)
        revealText =  tr("Reveal in Explorer");
#endif
        QAction* revealAction = new QAction(revealText, &menu);
        revealAction->setData( fileInfo.absoluteFilePath() );
        connect( revealAction, SIGNAL(triggered()), this, SLOT(revealItemInOSByAction()) );
        menu.addAction( revealAction );
    }


    // when the goto menu isn't empty add a seperator
    if( !gotoMenu->isEmpty() ) {
        gotoMenu->addSeparator();
    }
    gotoMenu->addAction( tr("/"), this, SLOT(setRootPath()) );


    menu.exec(globalPos);
}


/// When the rootPath is not given the root of the filesystem is used
/// @param rootPath the root path
void FileTreeSideWidget::setRootPath( const QString& rootPath )
{
    QModelIndex idx = fileTreeModel_->index( rootPath );
    fileTreeRef_->setRootIndex(idx);
    int index = pathComboRef_->findText(rootPath);
    if( index < 0 ) {

        // add the current item
        int idx = rootPathList_->rowCount() ;
        rootPathList_->insertRow( idx );
        rootPathList_->setData( rootPathList_->index(idx), rootPath );
        pathComboRef_->setCurrentIndex( idx );

    } else {
        pathComboRef_->setCurrentIndex(index);
    }

}


/// Sets the root path by extacting the path from the QAction data
void FileTreeSideWidget::setRootPathByAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if( action ) {
        setRootPath( action->data().toString() );
    }
}


/// clears the current root path
void FileTreeSideWidget::clearCurrentRootPath()
{
    if( pathComboRef_->currentIndex() > 0 ) {
        rootPathList_->removeRow( pathComboRef_->currentIndex() );
    }
}


/// clear all root paths in the combobox
/// (It keeps the root path of course)
void FileTreeSideWidget::clearAllRootPaths()
{   
    rootPathList_->removeRows( 1, rootPathList_->rowCount()-1 );
}


/// Reveals the given filename in the tree
/// In other words this method selects the given file in the filetree (if it exsists)
///
/// This method is very tricky. it tries to reveal the item immediately
/// Problem is that it is possible the given iem hasn't been loaded yet. When it hasn't been loaded yet
/// it loads the next direct and keeps revealing until the item is loaded
///
/// @param filename the filename to reveal
/// @return true if the file was completed (revealed or not) or false if the file needs to be loaded
///
bool FileTreeSideWidget::reveal(const QString& filename)
{
    QModelIndex idx = fileTreeModel_->index( filename );
    revealFileName_ = filename;

    // build the treePath
    QList<QModelIndex> treePath;
    treePath.append(idx);
    QModelIndex treeIdx = idx;
    while( treeIdx.isValid() ) {
        treePath.prepend(treeIdx);
        treeIdx= fileTreeModel_->parent(treeIdx);
    }


    // make sure the complete path has been loaded
    foreach( QModelIndex treeIdx, treePath ) {
        while( fileTreeModel_->canFetchMore(treeIdx) ) {
            fileTreeModel_->fetchMore(treeIdx);
            return false;
        }
    }

    // clear the reveal filename
    revealFileName_.clear();

    // stil a valid index
    if( idx.isValid() ) {
        fileTreeRef_->setCurrentIndex( idx );
        fileTreeRef_->scrollTo(idx);
        fileTreeRef_->expand(idx);
    }
    return true;
}


/// renames the given file-item
void FileTreeSideWidget::startRenameItem(const QString& filename)
{
    QModelIndex idx = fileTreeModel_->index( filename );
    if( idx.isValid() ) {
        fileTreeRef_->scrollTo( idx );
        fileTreeRef_->edit( idx );
    }
}


/// Starts the rename of a file with the help of a qaction object
void FileTreeSideWidget::startRenameItemByAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if( action ) {
        startRenameItem( action->data().toString() );
    }
}


/// Creates a new file and starts to edit this file
/// @param pathname the name of the path to create the file in
void FileTreeSideWidget::createNewFileAndRename(const QString& pathname)
{
    if( !pathname.isEmpty() ) {
        // generate a new filename
        QString filename = FileUtil().generateNewFilename( pathname, "Untitled %1.txt" );
        if( !filename.isEmpty() ) {

            // next go and create a 0 byte file
            QFile file(filename);
            if( file.open( QFile::WriteOnly) ) {
                file.flush();
                file.close();

                // edit this name of this file
                startRenameItem( filename );
            }
        }
    }
}


/// Creates a new file an renames it with the given action
void FileTreeSideWidget::createNewFileAndRenameByAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if( action ) {
        createNewFileAndRename( action->data().toString() );
    }
}


/// Creates a new folder and directly start renaming it
void FileTreeSideWidget::createNewFolderAndRename(const QString& pathname)
{
    if( !pathname.isEmpty() ) {
        // generate a new filename
        QString filename = FileUtil().generateNewFilename( pathname, "New Folder %1" );
        if( !filename.isEmpty() ) {

            // next go and create a folder
            if( QDir::root().mkdir(filename) ) {
                startRenameItem( filename );
            }
        }
    }
}


/// Creates a new folder and
void FileTreeSideWidget::createNewFolderAndRenameByAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if( action ) {
        createNewFolderAndRename( action->data().toString() );
    }
}


/// Duplicates the given file.  (It automaticly generates copy with a new unique filename)
/// Directly after doing this a rename file is started
/// @param pathname the original filename that needs duplication
void FileTreeSideWidget::duplicateFileAndRename(const QString& pathname)
{
    if( !pathname.isEmpty() ) {
        QFileInfo fileInfo(pathname);

        // get he file components
        QString path = fileInfo.path();
        QString filename = fileInfo.baseName();
        QString suffix = fileInfo.suffix();
        QString pattern = "%1/%2-%3%4";

        // when a hidden file is called (we append the number to the end
        if( filename.isEmpty() ) {
            pattern = "%1/%2%4-%3";
        }
        // make sure the suffix starts with a '.'
        if( !suffix.isEmpty() ) {
            suffix.prepend(".");
        }

        // TODO, support full directory structures. at the moment it copies a file :)

        // find th enew filename
        QFile file;
        int nr = 1;
        do
        {
            ++nr;
            file.setFileName( QString(pattern).arg(path).arg(filename).arg(nr).arg(suffix));
        }
        while( file.exists() );

        // now copy the file
        if( QFile::copy( pathname, file.fileName() ) ) {
            startRenameItem( file.fileName());
        }
    }
}


/// Duplicates a file with the given action
void FileTreeSideWidget::duplicateFileAndRenameByAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if( action ) {
        duplicateFileAndRename( action->data().toString() );
    }
}


/// Deletes the given file with the given pathname
/// @param pathname the item to delete
void FileTreeSideWidget::deleteItem(const QString& pathname)
{
    // when a pathname if given
    if( !pathname.isEmpty() ) {
        QFileInfo info( pathname );

        // ask for deletion
        if( QMessageBox::warning(this,
            tr("Delete %1?").arg(info.fileName()),
            tr("Delete %1 '%2'? Are you sure?").arg(info.isDir() ? "directory" : "file" ).arg(info.absoluteFilePath()),
            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,QMessageBox::Yes ) != QMessageBox::Yes ) {
            return;
        }

        // when it's a directory remove the dir if empty
        if( info.isDir() ) {
            QDir dir( info.absoluteFilePath() );
            dir.removeRecursively();

        // else delete the file
        } else {
            QFile::remove( pathname );
        }
    }
}


/// Deletes the given pathname with
void FileTreeSideWidget::deleteItemByAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if( action ) {
        deleteItem( action->data().toString() );
    }
}


/// This method reveals the given filetree item in OS File
void FileTreeSideWidget::revealItemInOSByAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if( action ) {
        FileUtil().revealInOSFileBrowser( action->data().toString() );
    }
}


/// The directory has been loaded
void FileTreeSideWidget::directoryLoaded(const QString& directory)
{
    // keep revealing until done :)
    if( revealFileName_.startsWith(directory) ) {
        reveal(revealFileName_);
    }
}


/// Constructs the user interface
void FileTreeSideWidget::constructUI()
{
    QFont newFont = QFont(font().family(), 11 );
    setFont(newFont);

    // create the tree model
    fileTreeModel_ = new QFileSystemModel();
    fileTreeModel_->setReadOnly(false);
    fileTreeModel_->setRootPath("/");
    fileTreeModel_->setFilter(QDir::Hidden|QDir::AllEntries|QDir::NoDotAndDotDot);
//    fileTreeModel_->setSorting( QDir::DirsFirst | QDir::IgnoreCase | QDir::Name );

    fileTreeRef_ = new QTreeView();
    fileTreeRef_->setContextMenuPolicy(Qt::CustomContextMenu);
    fileTreeRef_->setModel( fileTreeModel_ );
    fileTreeRef_->setFocusPolicy( Qt::ClickFocus );
    fileTreeRef_->setEditTriggers( QAbstractItemView::EditKeyPressed );

//    QHeaderView* hdr = fileTreeRef_->header();
//    hdr->setStretchLastSection(true);
//    hdr->setSortIndicator(0,Qt::AscendingOrder);
//    hdr->setSortIndicatorShown(true);
//    hdr->setSectionsClickable(true);

    QModelIndex index = fileTreeModel_->index(QDir::currentPath() );
    fileTreeRef_->expand(index);
    fileTreeRef_->scrollTo(index);
    fileTreeRef_->resizeColumnToContents(0);
    fileTreeRef_->setIndentation(15);

    // create the combobox with a small trash button
    pathComboRef_ = new QComboBox();
    pathComboRef_->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );

    // add trash button
    trashButtonRef_ = new QPushButton();
    trashButtonRef_->setIcon(  edbeeApp()->qtAwesome()->icon( icon_caret_down ) );
    trashButtonRef_->setFlat(true);
    trashButtonRef_->setContentsMargins(0,0,0,0);
    trashButtonRef_->setMaximumWidth(20);
    trashButtonRef_->setMaximumHeight(16);

    // add a menu for the trash button
    QMenu* menu = new QMenu(this);
    menu->addAction( tr("Clear Current Item"), this, SLOT(clearCurrentRootPath()) );
    menu->addAction( tr("Clear All"), this, SLOT(clearAllRootPaths()) );
    trashButtonRef_->setMenu(menu);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);

    // add the root paths combobox
    QHBoxLayout* comboLayout = new QHBoxLayout();
    comboLayout->setSpacing(0);
    comboLayout->addWidget(pathComboRef_,1);
    comboLayout->addWidget(trashButtonRef_,0);

    layout->addLayout( comboLayout, 0 );
    layout->addWidget( fileTreeRef_, 1 );
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout( layout);
}


/// Connects all signals
void FileTreeSideWidget::connectSignals()
{
    connect( fileTreeModel_, SIGNAL(directoryLoaded(QString)), this, SLOT(directoryLoaded(QString)) );
    connect( fileTreeRef_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openFileItem(QModelIndex)) );
    connect( fileTreeRef_, SIGNAL(activated(QModelIndex)), this, SLOT(openFileItem(QModelIndex)) );
    connect( fileTreeRef_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(fileTreeContextMenu(QPoint)) );
    connect( pathComboRef_, SIGNAL(currentIndexChanged(QString)), this, SLOT(setRootPath(QString)));
}

