/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "application.h"

#include <QAbstractNativeEventFilter>
#include <QDir>
#include <QFileOpenEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>

#include "commands/revealinsidebarcommand.h"
#include "edbee/edbee.h"
#include "edbee/io/tmlanguageparser.h"
#include "edbee/models/texteditorcommandmap.h"
#include "edbee/views/texttheme.h"
#include "io/appstateserializer.h"
#include "io/workspaceserializer.h"
#include "models/edbeeconfig.h"
#include "models/workspace.h"
#include "models/workspacemanager.h"
#include "QtAwesome.h"
#include "ui/mainwindow.h"
#include "ui/windowmanager.h"

#include "edbee/debug.h"


/// constructs the main appication object
/// @param argc the application argument acount
/// @param argv the application arguments
Application::Application(int& argc, char** argv )
    : QApplication( argc, argv )
    , qtAwesome_(0)
    , config_(0)
{
    config_ = new EdbeeConfig();
    windowManager_ = new WindowManager();
    workspaceManager_ = new WorkspaceManager();



//    connect( this, &Application::aboutToQuit, this, &Application::shutdown );
}


/// destruct the 'owned' objects
Application::~Application()
{
    delete workspaceManager_;
    delete windowManager_;
    delete config_;
    delete qtAwesome_;
}


/// Initializes the application
void Application::initApplication()
{
    // Initialize the application paths
    #ifdef Q_OS_MAC
        appDataPath_    = applicationDirPath() + "/../Resources/";
    #else
        appDataPath_ = qApp->applicationDirPath() + "/data/";
    #endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    userDataPath_   = QStandardPaths::writableLocation( QStandardPaths::DataLocation) + "/";
#else
    userDataPath_   = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/";
#endif

    // configure the edbee component to use the default paths
    edbee::Edbee* tm = edbee::Edbee::instance();
    tm->setKeyMapPath( QString("%1%2").arg(appDataPath_).arg("keymaps"));
    tm->setGrammarPath(  QString("%1%2").arg(appDataPath_).arg("syntaxfiles") );
    tm->setThemePath( QString("%1%2").arg(appDataPath_).arg("themes") );
    tm->init();   
    tm->autoShutDownOnAppExit();

    // register the custom command
    registerCustomEditorCommands();

    // make sure the user config path exists
//    QDir dir( userConfigPath() );
    QDir dir;
    dir.mkpath( userConfigPath() );

    // add the configuration paths to the edbeeconfig
    config()->addFile( QString("%1%2").arg(appConfigPath()).arg("default.json") );
    config()->addFile( QString("%1%2").arg(appConfigPath()).arg( QString("default.%1.json").arg(osNameString()) ), EdbeeConfig::Optional );
    config()->addFile( QString("%1%2").arg(userConfigPath()).arg("default.json"), EdbeeConfig::AutoCreate );
    config()->addFile( QString("%1%2").arg(userConfigPath()).arg( QString("default.%1.json").arg(osNameString()) ), EdbeeConfig::Optional );

    // load the configuration
    if( !config()->loadConfig() ) {

        /// build a nice error message
        QString messages;
        for( int i=0, cnt=config()->fileCount(); i<cnt; ++i ) {
            QString msg = config()->loadMessageForFile(i);
            if( !msg.isEmpty() ) {
                messages.append( tr("- %1: %2\n").arg(config()->file(i)).arg(msg) );
            }
        }
        // and show it
        if( !messages.isEmpty() ) {
            QString title = tr("Error loading configuration file(s)");
            QMessageBox::warning(0, title,QString("%1\n%2").arg(title).arg(messages) );
        }
    }


    // make qtawesome
    qtAwesome_ = new QtAwesome( this);
    qtAwesome_->initFontAwesome();

    // restore the last state
    loadState();

    // when there's no active session, just start a blank one
    if( workspaceManager()->size() == 0 ) {
        Workspace* workspace = workspaceManager()->createWorkspace();
        MainWindow* window = windowManager()->createWindow(workspace);
        window->show();
    }
}


/// thsi method shutsdown the application
void Application::shutdown()
{

}


/// Loads the last state of the application
void Application::loadState()
{
    AppStateSerializer io;
    if( !io.loadState(lastSessionFilename()) ) {
        qlog_warn() << "Error restoring session state to " << lastSessionFilename();
    }
}


/// This method saves the application state to the last session
/// When re-opening edbee the state is restored again
void Application::saveState()
{
    // serialize the previous state.
    AppStateSerializer appIO;
    if( !appIO.saveState(lastSessionFilename()) ) {
        qlog_warn() << "Error saving session state to " << lastSessionFilename() << ": " << appIO.errorMessage();
    }

    // when there's a filename in the workspace state. Also save that filename
    WorkspaceManager* workspaceManager = edbeeApp()->workspaceManager();
    for( int i=0,cnt=workspaceManager->size(); i<cnt; ++i ) {

        // save every workspace
        WorkspaceSerializer workspaceIO;
        Workspace* workspace = workspaceManager->workspace(i);
        if( !workspace->filename().isEmpty() ) {
            if( !workspaceIO.saveWorkspace( workspace) ) {
                qlog_warn() << "Error saving workspace " << workspace->filename() << ": " << workspaceIO.errorMessage();
            }
        }
    }
}


/// returns the qtAwesome instance for the application's icons
QtAwesome *Application::qtAwesome() const
{
    return qtAwesome_;
}


/// Returns the (default) icon font
QFont Application::iconFont(int size) const
{
    return qtAwesome()->font(size);
}


/// Returns the application data path.
/// This is the path to fixed application resources
/// On Mac OS X this will be in the .app 'file'
QString Application::appDataPath() const
{
    return appDataPath_;
}


/// Returns tha full application coniguration path
QString Application::appConfigPath() const
{
    return QString("%1%2/").arg(appDataPath()).arg("config");
}


/// Returns the user config data path. This is the data path where user specific
/// settings are stored
QString Application::userDataPath() const
{
    return userDataPath_;
}


/// Returns the full user configuration path
QString Application::userConfigPath() const
{
    return QString("%1%2/").arg(userDataPath()).arg("config");
}


/// Returns the filename that used to store/load the last session state
QString Application::lastSessionFilename() const
{
    return QString("%1%2").arg(userConfigPath()).arg("saved-session.json");
}


/// Returns the edbee configuration
EdbeeConfig* Application::config() const
{
    return config_;
}


/// Returns the application window manager
WindowManager* Application::windowManager() const
{
    return windowManager_;
}


/// Returns the workspace manager
WorkspaceManager* Application::workspaceManager() const
{
    return workspaceManager_;
}


/// This method returnst true if we're running on Mac OS X
/// Note: I could have used macro's but that will probably litter the application with #ifdefs
/// the current approach is much more flexible.
bool Application::isOSX()
{
#ifdef Q_OS_MAC
    return true;
#else
    return false;
#endif
}


/// This method reutrns true if we're running on a Unix X11 environment
bool Application::isX11()
{
#ifdef Q_OS_X11
    return true;
#else
    return false;
#endif
}


/// Returns true if the current environment is windows
bool Application::isWin()
{
#ifdef Q_OS_WIN32
    return true;
#else
    return false;
#endif
}


/// This method returns the operatingsystem string
/// that's can be used as prefix/postfix items in config-settings
const char *Application::osNameString()
{
#ifdef Q_OS_MAC
    return "osx";
#elif defined(Q_OS_WIN)
    return "win";
#else
    return "x11";
#endif

}


/// Returns the active workspace
/// @return the current active workspace
Workspace* Application::activeWorkspace() const
{
    return workspaceManager()->activeWorkspace();
}


/// This is the place where all (application) events pass by
/// unfortunaltely we need to handle an event here
bool Application::event(QEvent* event)
{
//    qlog_info()<< "event: " << event;
    switch (event->type())
    {
        case QEvent::FileOpen:
        {
            QString file = static_cast<QFileOpenEvent *>(event)->file();
            qlog_info() << "TODO: File open event: " <<file;
            //loadFile(static_cast<QFileOpenEvent *>(event)->file());
            return true;
        }

        // when we recieve a close event the state must be saved
        // Currently this is the only place I could find that could intercept the application quit before the windows are closed
        // We need to save the state before the windows close, else there's not much state left :D
        case QEvent::Close:
        {
            saveState();
            break;
        }
        default:
            break;
    }
    return QApplication::event(event);
}


/// Registers some extra editor commands
/// By registering commands to the TextEditorCommand map it becomes possible to assign keybinding to them
void Application::registerCustomEditorCommands()
{
    edbee::TextEditorCommandMap* map = edbee::Edbee::instance()->defaultCommandMap();
    map->give( "app.reveal-in-sidebar", new RevealInSidebarCommand() );
}


/// A global function to access the edbee application quickly
/// returns the Application instance
Application* edbeeApp()
{
    return static_cast<Application *>(qApp);
}


