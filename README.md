# simp-tar

This project is intended for creating a simple TAR archive, it uses CTarArchive class for that purpose; it's a header-only class using another header only class named CBuffType.

To create a TAR file you can use the following code snippet:

    CTarArchive tar(R"(c:\test\tfilex.tar)");

    tar.setMove(true);
    tar.addFileFromPath(R"(c:\test\a1.txt)");
    tar.addFileFromPath(R"(c:\test\a2.txt)");
    tar.close();

In previous example you create a file named ```tfile.tar``` with two files within. These files will be removed after creating the archive. Files won't be added with full path, if you want this to happen you can just modify the class accordingly. Another way to do it is the following:

    CTarArchive tar;
    struct stat fst;
    
    tar.setFileName(R"(c:\test\tfilex.tar)");
    tar.open();
    stat(R"(c:\test\a1.txt)", &fst);
    tar.addFile("a1.txt", fst.st_mtime);
    
    ifstream ifs(R"(c:\test\a1.txt)")
    CBuffType bt(65536, false);
    
    while (!ifs.eof())
    {
        ifs >> bt;
        tar.addData(std::string(bt.data, bt.actualRead));
    }
    ifs.close();
    tar.close();

The former snippet is intended for streaming into the file. The drawback of this way of creating a TAR archive is that there's needed space in system drive, because a %TEMP% file is created on every call to ```CTarArchive.addFile```; previous temporary file content is replaced after every call.

