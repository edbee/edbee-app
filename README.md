edbee-app
=========

Edbee (-app) is a developers text editor. It supports Multiple-carets, has Textmate Grammar and Highlighting support. I has multiple projects/workspaces and character encodings. 


The main website for edbee is at http://edbee.net/ 

You can find the generated (source) documentation at http://docs.edbee.net/


![Screenshot of the example application](http://edbee.net/images/screenshot2.png)

Originally this editor was used as an example application for the edbee library. 
The core of this editor if the edbee library: [https://github.com/edbee/edbee-lib](https://github.com/edbee/edbee-lib). 


Building edbee
---------------

This projects depends on 2 submodules:  
  * [https://github.com/edbee/edbee-lib](https://github.com/edbee/edbee-lib)
  * [https://github.com/edbee/edbee-data](https://github.com/edbee/edbee-data)

It also requires the Qt5 library.

Cloning edbee can be done with the following command:

```
git clone --recursive git://github.com/edbee/edbee-app.git
```

Building edbee should be as easy as opening edbee-app.pro in QtCreator and pressing build.

Command line building should also be possible. Usually by calling qmake followed by make. 
The exact commandline operation depends on the operating system and installed toolsets.

Known Issues and Missing Features
---------------------------------

* The editor doesn't support word-wrapping. (yet)
* It has issues with long lines. The cause of this is the nature of QTextLayout and the support of variable font sizes. In the future this can be fixed for monospaced fonts.
* Optimalisations for better render support and background calculate/paint-ahead functionality
* I really want to build in scripting support, for extending the editor with plugins. 


Contributing
------------

You can contribute via github
- Fork it
- Create your feature branch (git checkout -b my-new-feature)
- Commit your changes (git commit -am 'Added some feature')
- Push to the branch (git push origin my-new-feature)
- Create new Pull Request

Of course you can also contribute by contacting me via twitter @gamecreature or drop me al message 
via the email-address supplied at [https://github.com/gamecreature](https://github.com/gamecreature)

Issues?
-------

Though we have our own issue-tracker at ( http://issues.edbee.net/ ), you can report your problems 
via the github issue tracker or send me a message [https://github.com/gamecreature](https://github.com/gamecreature)




