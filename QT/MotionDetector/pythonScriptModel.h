#ifndef PYTHONSCRIPTMODEL_H
#define PYTHONSCRIPTMODEL_H
#include <python3.12/Python.h>
#include <QString>
#include <QDebug>
#include <QCoreApplication>
#include <conio.h>
class PythonScriptModel
{
public:
    PythonScriptModel();
    ~PythonScriptModel();

    void pythonTest();
    bool loadMotionDetectionModel(const QString &modelName);

    QString getCurrentModelName(){ return currentModelName; }
    bool isModelLoaded(){ return modelLoaded; }

private:
    QString currentModelName;
    bool modelLoaded;
    QString scriptsPath;
    void initializeScriptsPath();
};

void pytonScript();
#endif // PYTHONSCRIPTMODEL_H
