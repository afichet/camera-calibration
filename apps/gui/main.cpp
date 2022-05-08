#include "mainwindow.h"

#include <iostream>

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCommandLineParser>


enum CommandLineParseResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested
};

struct Command {
    bool    hasInputFile = false;
    QString inputFile;
};

CommandLineParseResult parseCommandLine(QCommandLineParser& parser, Command* result, QString* errorMessage)
{
    parser.setApplicationDescription(
"Spectral Viewer " + QApplication::applicationVersion() + "\n\n\
Application for displaying spectral and tristimulus images.\n\
\n\
Copyright (C) CNRS, INRIA 2020.\n\
This program comes with ABSOLUTELY NO WARRANTY; for details type `-licence'.\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions.\n"
);

    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    const QCommandLineOption helpOption(parser.addHelpOption());
    const QCommandLineOption versionOption(parser.addVersionOption());

    parser.addPositionalArgument("input file", "Sets file to process.");

    // Basic handling, parsing error & version & help
    if (!parser.parse(QCoreApplication::arguments())) {
        *errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(versionOption)) {
        return CommandLineVersionRequested;
    }

    if (parser.isSet(helpOption)) {
        return CommandLineHelpRequested;
    }

    // Input file
    const QStringList positionalArguments = parser.positionalArguments();

    if (positionalArguments.isEmpty()) {
        return CommandLineOk;
    }

    if (positionalArguments.size() > 1) {
        *errorMessage = "Several 'input file' arguments specified.";
        return CommandLineError;
    } else if (positionalArguments.size() == 1 && !positionalArguments.first().isEmpty()) {
        result->inputFile = positionalArguments.first();
        QFileInfo info(result->inputFile);
        result->hasInputFile = info.isFile();
    } else {
        result->hasInputFile = false;
    }

    return CommandLineOk;
}

int main(int argc, char* argv[])
{
    QApplication       a(argc, argv);
    MainWindow         w;
    QCommandLineParser parser;
    Command            command;
    QString            errorMessage;

    QApplication::setApplicationVersion("0.1");

    const CommandLineParseResult commandLineParseResult = parseCommandLine(parser, &command, &errorMessage);

    switch (commandLineParseResult) {
        case CommandLineVersionRequested:
            parser.showVersion();
            break;
        case CommandLineHelpRequested:
            parser.showHelp();
            break;
        case CommandLineError:
            std::cerr << errorMessage.toStdString() << std::endl;
            return -1;
        case CommandLineOk:
            break;
    }

    // Set theme
    QFile f(":/theme.qss");

    if (f.exists()) {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        a.setStyleSheet(ts.readAll());
    }

    w.show();

    if (command.hasInputFile) {
        w.openFile(command.inputFile);
    }

    return a.exec();
}
