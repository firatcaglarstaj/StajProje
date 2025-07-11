#include "pythonScriptModel.h"
#include <QDir>
#include <QFileInfo>

PythonScriptModel::PythonScriptModel() : currentModelName("none"), modelLoaded(false)
{
    initializeScriptsPath();
}

void PythonScriptModel:: initializeScriptsPath()
{
    scriptsPath = QDir::currentPath() + "/scripts";
    qDebug() << "path:" <<  scriptsPath;

    QDir scriptsDir(scriptsPath);
    if (!scriptsDir.exists()) {
        qDebug() << "Scripts dosyası eksik";
    }
}
    void PythonScriptModel::pythonTest()
{
    Py_Initialize();
    PyRun_SimpleString("print('Test')");

    // Scripts klasöründeki kodu uygulama
    QString pythonCode = QString("import sys\n"
                                 "sys.path.append('%1')\n"
                                 "print('Scripts dosyası:', '%1')\n"
                                 "try:\n"
                                 "    import motion_detection_model\n"
                                 "    print('başarı ile import edildi')\n"
                                 "except ImportError as e:\n"
                                 "    print('Import error:', e)\n"
                                 "except Exception as e:\n"
                                 "    print('Other error:', e)")
                             .arg(scriptsPath);

    // Python kodunu çalıştırma
    PyRun_SimpleString(pythonCode.toStdString().c_str());

    Py_Finalize();
    printf("\nPython testi tamamlandı");


}

bool PythonScriptModel::loadMotionDetectionModel(const QString &modelName)
{
    Py_Initialize();
    QString pythonCode = QString("import sys\n"
                                 "import os\n"
                                 "scripts_path = r'%1'\n"
                                 "print(f'Scripts yolu: {scripts_path}')\n"
                                 "sys.path.insert(0, scripts_path)\n"
                                 "print(f'Loading model: %2')\n"
                                 "try:\n"
                                 "    import motion_detection_model\n"
                                 "    print('Modül başarıyla eklendi')\n"
                                 "    result = motion_detection_model.load_model('%2')\n"
                                 "    if result:\n"
                                 "        print('Model yüklendi')\n"
                                 "        # config\n"
                                 "        config = motion_detection_model.get_model_config('%2')\n"
                                 "        if config:\n"
                                 "            print(f'Config: {config}')\n"
                                 "        # Model tavsiyeleri\n"
                                 "        rec = motion_detection_model.get_performance_recommendations('%2')\n"
                                 "        print(f'Model Tavsiyeleri: {rec}')\n"
                                 "    else:\n"
                                 "        print('HATA')\n"
                                 "except ImportError as e:\n"
                                 "    print(f'IMPORT HATA: {e}')\n"
                                 "except Exception as e:\n"
                                 "    print(f'ERROR: {e}')\n")
                             .arg(scriptsPath).arg(modelName);


    // Python kodunu çalıştırma
    PyRun_SimpleString(pythonCode.toStdString().c_str());
    Py_Finalize();
    currentModelName = modelName;
    modelLoaded = true;
    return true;
}














