#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFile>
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <QPixmap>
#include <QApplication>
#include <QXmlStreamReader>
#include <QInputDialog>
#include <chosewavetoextract.h>
#include <QImage>

#include <QTimer>
#include <QElapsedTimer>

#include <QIODevice>
#include <QWidget>

#include <cmath>

#include <__common.h>
//#include "lstStructs.h"

//QString rootPath;
using namespace std;
//#define _CURRENT_DIR rootPath
#define _PROJECT_NAME "/hsi_app"
#define _PATH_SETTINGS_EM_ITERATIONS "/settings/Wavelengths/EMIterations.hypcam"
#define SQUARE_BICUBIC_ITERATIONS 1
#define _PATH_TMP_HYPCUBES              "./tmpHypCubes/"
#define _PATH_WAVE_CHOISES              "/settings/waveChoises.hypcam"
#define _PATH_CALIBRATION_FILE "/XML/hypcalib.xml"
#define _PATH_DISPLAY_IMAGE "/tmpImages/tmpImg2Disp.png"
#define _ERROR_FILE_NOTEXISTS           "~FileDoesNotExists"
#define _ERROR_FILE                     "~UnknowError"


#define _RED                            1
#define _GREEN                          2
#define _BLUE                           3
#define _RGB                            7

#define _OK                             1
#define _FAILURE                        0
#define _ERROR                          -1

//typedef struct strAllLinReg{
//       double horizA;
//       double horizB;
//       double vertA;
//       double vertB;
//       double waveHorizA;
//       double waveHorizB;
//       double waveVertA;
//       double waveVertB;
//       double deltaHorizA;
//       double deltaHorizB;
//       double deltaVertA;
//       double deltaVertB;
//   }strAllLinReg;


//typedef struct lstDoubleAxisCalibration{
//        QString         bkgPath;
//        int             W;
//        int             H;
//        float           bigX;
//        float           bigY;
//        float           bigW;
//        float           bigH;
//        float           squareX;
//        float           squareY;
//        float           squareW;
//        float           squareH;
//        int             squarePixX;
//        int             squarePixY;
//        int             squarePixW;
//        int             squarePixH;
//        int             squareUsableX;      //Respect to the snapshot
//        int             squareUsableY;      //Respect to the snapshot
//        int             squareUsableW;      //Respect to the snapshot
//        int             squareUsableH;      //Respect to the snapshot
//        float           minWavelength;
//        float           maxWavelength;
//        int             maxNumBands;
//        double          minSpecRes;
//        strAllLinReg    LR;
//        QList<double>   Sr;
//        QList<double>   Sg;
//        QList<double>   Sb;
//    }lstDoubleAxisCalibration;

typedef struct trilinear{
    int l;
    int L;
    int w;
    int W;
    int h;
    int H;
}trilinear;

typedef struct pixel{
    int x;
    int y;
    int index;
}pixel;

typedef struct strDiffProj{
    int x;
    int y;
    int rx;
    int ry;
    int ux;
    int uy;
    int lx;
    int ly;
    int dx;
    int dy;
    float wavelength;
}strDiffProj;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



//void funcShowMsg(QString title, QString msg)
//{
//    QMessageBox yesNoMsgBox;
//    yesNoMsgBox.setWindowTitle(title);
//    yesNoMsgBox.setText(msg);
//    yesNoMsgBox.setDefaultButton(QMessageBox::Ok);
//    yesNoMsgBox.exec();
//}


//void funcShowMsgERROR(QString msg)
//{
//    QMessageBox yesNoMsgBox;
//    yesNoMsgBox.setWindowTitle("ERROR");
//    yesNoMsgBox.setText(msg);
//    yesNoMsgBox.setDefaultButton(QMessageBox::Ok);
//    yesNoMsgBox.exec();
//}


//QString readAllFile(QString filePath)
//{
//    QFile tmpFile(filePath);
//    if (tmpFile.exists())
//    {
//        tmpFile.open(QIODevice::ReadOnly);
//        QTextStream tmpStream(&tmpFile);
//        return tmpStream.readAll();
//    }
//    else
//    {
//        return _ERROR_FILE_NOTEXISTS;
//    }
//    return _ERROR_FILE;
//}

int fileIsValid(QString fileContain)
{
    // return:
    //  1: exists and it is not empty
    // -1: empty file
    // -2: error reading
    // -3: it does note exist
    if (fileContain.isEmpty())
    {
        return -1;
    }
    if (fileContain.contains(_ERROR_FILE))
    {
        return -2;
    }
    if (fileContain.contains(_ERROR_FILE_NOTEXISTS))
    {
        return -3;
    }
    return 1;
}

int fileExists(QString fileName)
{
    //Receive a filename and return
    //1: file exists
    //0: file does not exist
    //-1: fileName empty
    //-2: Unknow error
    if (fileName.isEmpty())
        return -1;

    QFile tmpFile(fileName);
    if (tmpFile.exists())
        return 1;
    else
        return 0;

    return -2;
}

//bool saveFile(const QString &fileName, QString contain)
//{
//    QFile file(fileName);
//    if (file.exists())
//    {
//        if (!file.remove())
//        {
//            funcShowMsgERROR("Removing file: " + fileName);
//            return false;
//        }
//    }
//    if (file.open(QIODevice::ReadWrite))
//    {
//        QTextStream stream(&file);
////        stream << contain <<endl;
//        stream << contain <<"\n";
//        file.close();
//    }
//    else
//    {
//        funcShowMsgERROR("Opening file: " + fileName);
//        return false;
//    }
//    return true;
//}


QString readFileParam(const QString &filePath, const QString &pathDefault, int *OK)
{
    *OK = _ERROR;
    QString fileContain;
    fileContain.clear();
    if (fileExists(filePath))
    {
        fileContain = readAllFile(filePath);
        if (fileIsValid(fileContain))
        {
            fileContain = fileContain.trimmed();
            fileContain.replace("\n", "");
        }
        else
        {
            fileContain = pathDefault;
        }
    }
    else
    {
        saveFile(filePath, pathDefault);
        *OK = _FAILURE;
    }
    *OK = _OK;
    return fileContain;
}


int readFileParam(QString fileName, QString *tmpFileContain)
{
    *tmpFileContain = "";
    if (fileExists(fileName))
    {
        *tmpFileContain = readAllFile(fileName);
        if (fileIsValid(*tmpFileContain))
        {
            *tmpFileContain = tmpFileContain->trimmed();
            tmpFileContain->replace("\n", "");
        }
        else
        {
            return _ERROR;
        }
    }
    else
    {
        saveFile(fileName, *tmpFileContain);
        return _FAILURE;
    }
    return _OK;
}

QString readFileParam(QString fileName)
{
    QString tmpFileContain = "";
    if (fileExists(fileName))
    {
        tmpFileContain = readAllFile(fileName);
        if (fileIsValid(tmpFileContain))
        {
            tmpFileContain = tmpFileContain.trimmed();
            tmpFileContain.replace("\n", "");
        }
    }
    else
    {
        saveFile(fileName, tmpFileContain);
    }
    return tmpFileContain;
}

//void MainWindow::on_actionGenHypercube_triggered()
//{


//}

//void funcQStringToSensitivities(QString txt, QList<double> *p)
//{
//    int i;
//    QList<QString> lstSensitivities;
//    lstSensitivities = txt.split(",");
//    //p = (double*)malloc(lstSensitivities.count()*sizeof(double));
//    for (i = 0; i < lstSensitivities.count(); i++)
//    {
//        p->append(lstSensitivities.at(i).toDouble(0));
//    }
//}


QList<double> getWavesChoised()
{
    QList<double> wavelengths;
    QString waves;
    waves = readFileParam(rootpath()+_PROJECT_NAME+_PATH_WAVE_CHOISES);
    QList<QString> choises;
    choises = waves.split(",");
    Q_FOREACH (const QString choise, choises)
    {
        if (!choise.isEmpty() && choise != " " && choise != "\n")
        {
            wavelengths.append(choise.toDouble(0));
        }
    }
    return wavelengths;
}

//bool funcGetCalibration(lstDoubleAxisCalibration *doubAxisCal)
//{

//    QFile *xmlFile = new QFile(_CURRENT_DIR+_PROJECT_NAME+_PATH_CALIBRATION_FILE);
//    if (!xmlFile->open(QIODevice::ReadOnly | QIODevice::Text))
//    {
//        funcShowMsg("ERROR", "Opening _CURRENT_DIR+_PROJECT_NAME+_PATH_CALIBRATION_FILE");
//        return false;
//    }
//    QXmlStreamReader *xmlReader = new QXmlStreamReader(xmlFile);

//    //Parse the XML until we reach end of it
//    while (!xmlReader->atEnd() && !xmlReader->hasError())
//    {
//         //Read next element
//        QXmlStreamReader::TokenType token = xmlReader->readNext();
//        //If token is just StartDocument - go to next
//        if (token == QXmlStreamReader::StartDocument)
//        {
//            continue;


//        }
//        //If token is StartElement - read it
//        if (token == QXmlStreamReader::StartElement)
//        {

//            if (xmlReader->name().toString() == "bkgPath")
//                doubAxisCal->bkgPath = xmlReader->readElementText();



//            if (xmlReader->name().toString() == "W")
//                doubAxisCal->W = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "H")
//                doubAxisCal->H = xmlReader->readElementText().toInt(0);

//            if (xmlReader->name().toString() == "bigX")
//                doubAxisCal->bigX = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "bigY")
//                doubAxisCal->bigY = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "bigW")
//                doubAxisCal->bigW = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "bigH")
//                doubAxisCal->bigH = xmlReader->readElementText().toFloat(0);

//            if (xmlReader->name().toString() == "squareX")
//                doubAxisCal->squareX = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "squareY")
//                doubAxisCal->squareY = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "squareW")
//                doubAxisCal->squareW = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "squareH")
//                doubAxisCal->squareH = xmlReader->readElementText().toFloat(0);

//            if (xmlReader->name().toString() == "squarePixX")
//                doubAxisCal->squarePixX = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "squarePixY")
//                doubAxisCal->squarePixY = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "squarePixW")
//                doubAxisCal->squarePixW = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "squarePixH")
//                doubAxisCal->squarePixH = xmlReader->readElementText().toInt(0);

//            if (xmlReader->name().toString() == "squareUsablePixX")
//                doubAxisCal->squareUsableX = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "squareUsablePixY")
//                doubAxisCal->squareUsableY = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "squareUsablePixW")
//                doubAxisCal->squareUsableW = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "squareUsablePixH")
//                doubAxisCal->squareUsableH = xmlReader->readElementText().toInt(0);

//            if (xmlReader->name().toString() == "horizontalA")
//                doubAxisCal->LR.horizA = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "horizontalB")
//                doubAxisCal->LR.horizB = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "verticalA")
//                doubAxisCal->LR.vertA = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "verticalB")
//                doubAxisCal->LR.vertB = xmlReader->readElementText().toFloat(0);

//            if (xmlReader->name().toString() == "waveHorizA")
//                doubAxisCal->LR.waveHorizA = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "waveHorizB")
//                doubAxisCal->LR.waveHorizB = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "waveVertA")
//                doubAxisCal->LR.waveVertA = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "waveVertB")
//                doubAxisCal->LR.waveVertB = xmlReader->readElementText().toFloat(0);

//            if (xmlReader->name().toString() == "deltaHorizA")
//                doubAxisCal->LR.deltaHorizA = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "deltaHorizB")
//                doubAxisCal->LR.deltaHorizB = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "deltaVertA")
//                doubAxisCal->LR.deltaVertA = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "deltaVertB")
//                doubAxisCal->LR.deltaVertB = xmlReader->readElementText().toFloat(0);

//            if (xmlReader->name().toString() == "minWavelength")
//                doubAxisCal->minWavelength = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "maxWavelength")
//                doubAxisCal->maxWavelength = xmlReader->readElementText().toFloat(0);
//            if (xmlReader->name().toString() == "maxNumBand")
//                doubAxisCal->maxNumBands = xmlReader->readElementText().toInt(0);
//            if (xmlReader->name().toString() == "minSpecRes")
//                doubAxisCal->minSpecRes = xmlReader->readElementText().toFloat(0);

//            if (xmlReader->name().toString() == "Sr")
//                funcQStringToSensitivities(xmlReader->readElementText(), &doubAxisCal->Sr);
//            if (xmlReader->name().toString() == "Sg")
//                funcQStringToSensitivities(xmlReader->readElementText(), &doubAxisCal->Sg);
//            if (xmlReader->name().toString() == "Sb")
//                funcQStringToSensitivities(xmlReader->readElementText(), &doubAxisCal->Sb);

//        }
//    }
//    if (xmlReader->hasError())
//    {
//        funcShowMsg("_PATH_STARTING_SETTINGS Parse Error", xmlReader->errorString());
//    }
//    xmlReader->clear();
//    xmlFile->close();

//    return true;
//}

double calcTrilinearInterpolation(double ***cube, trilinear *node)
{

    double result = cube[node->l][node->h][node->w];
    if (
        node->l > 0 && node->l < node->L - 1 &&
        node->w > 0 && node->w < node->W - 1 &&
        node->h > 0 && node->h < node->H - 1)
    {
        result = (cube[node->l - 1][node->h - 1][node->w - 1] +
                  cube[node->l - 1][node->h - 1][node->w] +
                  cube[node->l - 1][node->h - 1][node->w + 1] +
                  cube[node->l - 1][node->h][node->w - 1] +
                  cube[node->l - 1][node->h][node->w] +
                  cube[node->l - 1][node->h][node->w + 1] +
                  cube[node->l - 1][node->h + 1][node->w - 1] +
                  cube[node->l - 1][node->h + 1][node->w] +
                  cube[node->l - 1][node->h + 1][node->w + 1] +

                  cube[node->l][node->h - 1][node->w - 1] +
                  cube[node->l][node->h - 1][node->w] +
                  cube[node->l][node->h - 1][node->w + 1] +
                  cube[node->l][node->h][node->w - 1] +
                  cube[node->l][node->h][node->w] +
                  cube[node->l][node->h][node->w + 1] +
                  cube[node->l][node->h + 1][node->w - 1] +
                  cube[node->l][node->h + 1][node->w] +
                  cube[node->l][node->h + 1][node->w + 1] +

                  cube[node->l + 1][node->h - 1][node->w - 1] +
                  cube[node->l + 1][node->h - 1][node->w] +
                  cube[node->l + 1][node->h - 1][node->w + 1] +
                  cube[node->l + 1][node->h][node->w - 1] +
                  cube[node->l + 1][node->h][node->w] +
                  cube[node->l + 1][node->h][node->w + 1] +
                  cube[node->l + 1][node->h + 1][node->w - 1] +
                  cube[node->l + 1][node->h + 1][node->w] +
                  cube[node->l + 1][node->h + 1][node->w + 1]

                  ) /
                 27.0;
    }
    else
    {
        if (
            (node->l == 0 || node->w == 0 || node->h == 0) &&
            (node->l < node->L - 1 && node->w < node->W - 1 && node->h < node->H - 1))
        {
            result = (cube[node->l][node->h][node->w] +
                      cube[node->l][node->h][node->w + 1] +
                      cube[node->l][node->h + 1][node->w] +
                      cube[node->l][node->h + 1][node->w + 1] +

                      cube[node->l + 1][node->h][node->w] +
                      cube[node->l + 1][node->h][node->w + 1] +
                      cube[node->l + 1][node->h + 1][node->w] +
                      cube[node->l + 1][node->h + 1][node->w + 1]

                      ) /
                     8.0;
        }
        else
        {
            if (
                (node->l == node->L - 1 || node->w == node->W - 1 || node->h == node->H - 1) &&
                (node->l > 0 && node->w > 0 && node->h > 0))
            {
                result = (cube[node->l][node->h][node->w] +
                          cube[node->l][node->h][node->w - 1] +
                          cube[node->l][node->h - 1][node->w] +
                          cube[node->l][node->h - 1][node->w - 1] +

                          cube[node->l - 1][node->h][node->w] +
                          cube[node->l - 1][node->h][node->w - 1] +
                          cube[node->l - 1][node->h - 1][node->w] +
                          cube[node->l - 1][node->h - 1][node->w - 1]

                          ) /
                         8.0;
            }
        }
    }

//    from here
//    double result;
//    if(node->l>0 && node->l<node->L-1)
//    {
//        //qDebug() << "l: " << node->l << " h: " << node->h << " w: " << node->w;
//        result = (
//                    cube[node->l-1][node->h][node->w] +
//                    cube[node->l+1][node->h][node->w]
//                 ) / 2.0;
//    }
//    else
//    {
//        if(node->l==0)
//        {
//            result = (
//                        cube[node->l][node->h][node->w] +
//                        cube[node->l+1][node->h][node->w]
//                     ) / 2.0;
//        }
//        else
//        {
//            result = (
//                        cube[node->l][node->h][node->w] +
//                        cube[node->l-1][node->h][node->w]
//                     ) / 2.0;
//        }
//    }



//    if(
//            node->l > 0 && node->l < node->L-1 &&
//            node->w > 0 && node->w < node->W-1 &&
//            node->h > 0 && node->h < node->H-1
//    )
//    {//BOUNDED BY THE MARGIN
//        if(true)
//        {
//            result = (  cube[node->l][node->h-1][node->w-1]     + cube[node->l][node->h-1][node->w+1]     +
//                        cube[node->l][node->h+1][node->w-1]     + cube[node->l][node->h+1][node->w+1]     +
//                        cube[node->l-1][node->h-1][node->w-1]   + cube[node->l-1][node->h-1][node->w+1]   +
//                        cube[node->l-1][node->h+1][node->w-1]   + cube[node->l-1][node->h+1][node->w+1]   +
//                        cube[node->l+1][node->h-1][node->w-1]   + cube[node->l+1][node->h-1][node->w+1]   +
//                        cube[node->l+1][node->h+1][node->w-1]   + cube[node->l+1][node->h+1][node->w+1]
//                     ) / 12.0;
//        }
//        else
//        {
//            result = (
//                        cube[node->l-1][node->h-1][node->w-1]   + cube[node->l-1][node->h-1][node->w+1]   +
//                        cube[node->l-1][node->h+1][node->w-1]   + cube[node->l-1][node->h+1][node->w+1]   +
//                        cube[node->l+1][node->h-1][node->w-1]   + cube[node->l+1][node->h-1][node->w+1]   +
//                        cube[node->l+1][node->h+1][node->w-1]   + cube[node->l+1][node->h+1][node->w+1]
//                     ) / 8.0;
//        }
//    }
//    else
//    {
//        result = cube[node->l][node->h][node->w];
//    }


//    till here

    return result;
}


double *demosaiseF2D(double *f, int L, int H, int W)
{
    //Variables
    int i, l, w, h;
    double ***aux;
    aux = (double ***)malloc(L * sizeof(double **));
    for (l = 0; l < L; l++)
    {
        aux[l] = (double **)malloc(H * sizeof(double *));
        for (h = 0; h < H; h++)
        {
            aux[l][h] = (double *)malloc(W * sizeof(double));
        }
    }

    //Fill as 3D matrix
    i = 0;
    for (l = 0; l < L; l++)
    {
        for (h = 0; h < H; h++)
        {
            for (w = 0; w < W; w++)
            {
                aux[l][h][w] = f[i];
                i++;
            }
        }
    }

    //Demosaize
    i = 0;
    for (l = 0; l < L; l++)
    {
        for (h = 0; h < H; h++)
        {
            for (w = 0; w < W; w++)
            {
                if (h > 0 && w > 0 && h < H - 1 && w < W - 1)
                {
                    f[i] = (aux[l][h - 1][w - 1] +
                            aux[l][h + 1][w - 1] +
                            aux[l][h - 1][w + 1] +
                            aux[l][h + 1][w + 1]) /
                           4.0;
                }
                else
                {
                    //ROWS
                    if (h == 0 || h == (H - 1))
                    {
                        if (h == 0) //First row
                        {
                            if (w > 0 && w < (W - 1))
                                f[i] = (aux[l][h + 1][w - 1] + aux[l][h + 1][w + 1]) / 2.0;
                            else //Corners
                            {
                                if (w == 0) //Up-Left
                                    f[i] = (aux[l][h + 1][w] + aux[l][h + 1][w + 1] + aux[l][h][w + 1]) / 3.0;
                                if (w == (W - 1)) //Up-Right
                                    f[i] = (aux[l][h][w - 1] + aux[l][h + 1][w - 1] + aux[l][h + 1][w]) / 3.0;
                            }
                        }
                        else
                        {
                            if (h == (H - 1)) //Last row
                            {
                                if (w > 0 && w < (W - 1))
                                    f[i] = (aux[l][h - 1][w - 1] + aux[l][h - 1][w + 1]) / 2.0;
                                else //Corners
                                {
                                    if (w == 0) //Down-Left
                                        f[i] = (aux[l][h - 1][w] + aux[l][h - 1][w + 1] + aux[l][h][w + 1]) / 3.0;
                                    if (w == (W - 1)) //Down-Right
                                        f[i] = (aux[l][h][w - 1] + aux[l][h - 1][w - 1] + aux[l][h - 1][w]) / 3.0;
                                }
                            }
                            else
                            {
                                //COLS
                                if (w == 0 || w == (W - 1))
                                {
                                    if (w == 0)
                                        f[i] = (aux[l][h - 1][w + 1] + aux[l][h + 1][w + 1]) / 2.0;
                                    else
                                        f[i] = (aux[l][h - 1][w - 1] + aux[l][h + 1][w - 1]) / 2.0;
                                }
                            }
                        }
                    }
                }
                i++;
            }
        }
    }

    //Free memory
    for (l = 0; l < L; l++)
    {
        for (h = 0; h < H; h++)
        {
            delete[] aux[l][h];
        }
        delete[] aux[l];
    }
    delete[] aux;

    return f;
}

double *demosaiseF3D(double *f, int L, int H, int W)
{
    f = demosaiseF2D(f, L, H, W);

    //...............................................
    //Variables
    //...............................................
    int i, l, w, h;
    double ***aux;
    aux = (double ***)malloc(L * sizeof(double **));
    for (l = 0; l < L; l++)
    {
        aux[l] = (double **)malloc(H * sizeof(double *));
        for (h = 0; h < H; h++)
        {
            aux[l][h] = (double *)malloc(W * sizeof(double));
        }
    }

    //...............................................
    //Fill a 3D matrix
    //...............................................
    i = 0;
    for (l = 0; l < L; l++)
    {
        for (h = 0; h < H; h++)
        {
            for (w = 0; w < W; w++)
            {
                aux[l][h][w] = f[i];
                i++;
            }
        }
    }

    //...............................................
    //Demosaize 3D
    //...............................................
    trilinear tmpNode;
    tmpNode.L = L;
    tmpNode.W = W;
    tmpNode.H = H;
    i = 0;
    for (tmpNode.l = 0; tmpNode.l < L; tmpNode.l++)
    {
        for (tmpNode.h = 0; tmpNode.h < H; tmpNode.h++)
        {
            for (tmpNode.w = 0; tmpNode.w < W; tmpNode.w++)
            {
                //qDebug() << "i: " << i << "l: " << tmpNode.l << "w: " << tmpNode.w << "h: " << tmpNode.h << "W: " << tmpNode.W << "H: " << tmpNode.H;
                f[i] = calcTrilinearInterpolation(aux, &tmpNode);
                i++;
            }
        }
    }

    //...............................................
    //Free memory
    //...............................................
    for (l = 0; l < L; l++)
    {
        for (h = 0; h < H; h++)
        {
            delete[] aux[l][h];
        }
        delete[] aux[l];
    }
    delete[] aux;

    //Finishes
    return f;
}


void improveF(double *fKPlusOne, pixel **Hcol, double *f, double *gTmp, int N)
{
    int n;
    double avgMeasure; //average measure
    double relevance;  //How relevant it is respect to all voxels overlaped
                       //Error between g and g^ (g = original image)
    double numProj;    //It is integer but is used double to evit many cast operations
    numProj = 5.0;
    for (n = 0; n < N; n++)
    {
        //fKPlusOne[n]    = 0.0;
        avgMeasure = f[n] / numProj;
        //qDebug() << "n: " << n << " | avgMeasure: " << avgMeasure;

        relevance = gTmp[Hcol[n][0].index] +
                    gTmp[Hcol[n][1].index] +
                    gTmp[Hcol[n][2].index] +
                    gTmp[Hcol[n][3].index] +
                    gTmp[Hcol[n][4].index];
        //qDebug() << "relevance: " << relevance;

        fKPlusOne[n] = avgMeasure * relevance;
        //qDebug() << "fKPlusOne[" << n << "]: " << fKPlusOne[n];
    }
}

void createsGTmp(double *gTmp, double *g, int **Hrow, double *f, int M)
{
    int m, n;
    for (m = 0; m < M; m++)
    {
        gTmp[m] = 0.0;
        //if( Hrow[m][0] > 0 )
        if (Hrow[m][0] != 0)
        {
            for (n = 1; n <= Hrow[m][0]; n++)
            {
                gTmp[m] += f[Hrow[m][n]];
            }
            //gTmp[m] = ( g[m] > 0 && gTmp[m] > 0 )?(g[m]/gTmp[m]):0;
            gTmp[m] = (g[m] != 0 && gTmp[m] != 0) ? (g[m] / gTmp[m]) : 0;
        }
    }
}

double *createsF0(pixel **Hcol, double *g, int N)
{
    double *f;
    f = (double *)malloc(N * sizeof(double));
    for (int n = 0; n < N; n++)
    {
        f[n] = g[Hcol[n][0].index] + //Zero
               g[Hcol[n][1].index] + //Right
               g[Hcol[n][2].index] + //Up
               g[Hcol[n][3].index] + //Left
               g[Hcol[n][4].index];  //Down
        //qDebug() << "f[" << n << "]: " << f[n];
    }
    return f;
}

double *serializeImageToProccess(QImage img, int sensor)
{
    int M, m;
    double *g;
    M = img.width() * img.height();
    g = (double *)malloc(M * sizeof(double));

    QRgb rgb;
    m = 0;
    for (int r = 0; r < img.height(); r++)
    {
        for (int c = 0; c < img.width(); c++)
        {
            if (sensor == _RED)
            {
                rgb = img.pixel(QPoint(c, r));
                g[m] = (double)qRed(rgb);
            }
            else
            {
                if (sensor == _RGB)
                {
                    rgb = img.pixel(QPoint(c, r));
                    g[m] = (double)(qRed(rgb) + qGreen(rgb) + qBlue(rgb));
                }
                else
                {
                    if (sensor == _GREEN)
                    {
                        rgb = img.pixel(QPoint(c, r));
                        g[m] = (double)qGreen(rgb);
                    }
                    else
                    { //_BLUE
                        rgb = img.pixel(QPoint(c, r));
                        g[m] = (double)qBlue(rgb);
                    }
                }
            }
            m++;
        }
    }
    return g;
}

int xyToIndex(int x, int y, int w)
{
    return ((y - 1) * w) + x;
}

void insertItemIntoRow(int **Hrow, int row, int col)
{
    int actualPos;
    actualPos = Hrow[row][0] + 1;
    Hrow[row] = (int *)realloc(Hrow[row], ((actualPos + 1) * sizeof(int)));
    Hrow[row][actualPos] = col;
    Hrow[row][0]++;
}

void calcDiffProj(strDiffProj *diffProj, lstDoubleAxisCalibration *daCalib)
{
    //int offsetX, offsetY;
    int origX, origY;

    origX = diffProj->x + daCalib->squareUsableX;
    origY = diffProj->y + daCalib->squareUsableY;

    //offsetX = abs( daCalib->squareUsableX - origX );
    //offsetY = abs( daCalib->squareUsableY - origY );

    //It calculates the jump
    int jumpX, jumpY;
    jumpX = floor(daCalib->LR.waveHorizA + (daCalib->LR.waveHorizB * diffProj->wavelength));
    jumpY = floor(daCalib->LR.waveVertA + (daCalib->LR.waveVertB * diffProj->wavelength));

    //Right
    diffProj->ry = floor(daCalib->LR.horizA + (daCalib->LR.horizB * (double)(origX + jumpX))) + diffProj->y;

    //Left
    diffProj->ly = floor(daCalib->LR.horizA + (daCalib->LR.horizB * (double)(origX - jumpX))) + diffProj->y;

    //Up
    diffProj->ux = floor(daCalib->LR.vertA + (daCalib->LR.vertB * (double)(origY - jumpY))) + diffProj->x;

    //Down
    diffProj->dx = floor(daCalib->LR.vertA + (daCalib->LR.vertB * (double)(origY + jumpY))) + diffProj->x;

    //Fits the original "y"
    diffProj->y = floor(daCalib->LR.horizA + (daCalib->LR.horizB * (double)origX)) + diffProj->y;
    diffProj->x = floor(daCalib->LR.vertA + (daCalib->LR.vertB * (double)origY)) + diffProj->x;

    diffProj->rx = diffProj->x + jumpX;
    diffProj->lx = diffProj->x - jumpX;
    diffProj->uy = diffProj->y - jumpY;
    diffProj->dy = diffProj->y + jumpY;
}

void createsHColAndHrow(pixel **Hcol, int **Hrow, QImage *img, lstDoubleAxisCalibration *daCalib)
{
    //Prepares variables and constants
    //..
    int hypW, hypH, hypL, idVoxel;
    QList<double> lstChoises;
    strDiffProj Pj;
    lstChoises = getWavesChoised();
    hypW = daCalib->squareUsableW;
    hypH = daCalib->squareUsableH;
    hypL = lstChoises.count();

    //Fill Hcol
    //..
    idVoxel = 0;
    for (int len = 1; len <= hypL; len++)
    {
        Pj.wavelength = lstChoises.at(len - 1);
        for (int row = 1; row <= hypH; row++)
        {
            for (int col = 1; col <= hypW; col++)
            {
                //Obtain diffraction projection for the acutual wavelength
                Pj.x = col;
                Pj.y = row;
                calcDiffProj(&Pj, daCalib);
                //Creates a new item in the c-th Hcol
                Hcol[idVoxel][0].x = Pj.x; //Zero
                Hcol[idVoxel][0].y = Pj.y;
                Hcol[idVoxel][0].index = xyToIndex(Hcol[idVoxel][0].x, Hcol[idVoxel][0].y, img->width());

                Hcol[idVoxel][1].x = Pj.rx; //Right
                Hcol[idVoxel][1].y = Pj.ry;
                Hcol[idVoxel][1].index = xyToIndex(Hcol[idVoxel][1].x, Hcol[idVoxel][1].y, img->width());

                Hcol[idVoxel][2].x = Pj.ux; //Up
                Hcol[idVoxel][2].y = Pj.uy;
                Hcol[idVoxel][2].index = xyToIndex(Hcol[idVoxel][2].x, Hcol[idVoxel][2].y, img->width());

                Hcol[idVoxel][3].x = Pj.lx; //Left
                Hcol[idVoxel][3].y = Pj.ly;
                Hcol[idVoxel][3].index = xyToIndex(Hcol[idVoxel][3].x, Hcol[idVoxel][3].y, img->width());

                Hcol[idVoxel][4].x = Pj.dx; //Down
                Hcol[idVoxel][4].y = Pj.dy;
                Hcol[idVoxel][4].index = xyToIndex(Hcol[idVoxel][4].x, Hcol[idVoxel][4].y, img->width());

                //Creates new item in Hrow
                insertItemIntoRow(Hrow, Hcol[idVoxel][0].index, idVoxel);
                insertItemIntoRow(Hrow, Hcol[idVoxel][1].index, idVoxel);
                insertItemIntoRow(Hrow, Hcol[idVoxel][2].index, idVoxel);
                insertItemIntoRow(Hrow, Hcol[idVoxel][3].index, idVoxel);
                insertItemIntoRow(Hrow, Hcol[idVoxel][4].index, idVoxel);

                idVoxel++;
            }
        }
    }
}


double *calculatesF(int numIterations, int sensor, lstDoubleAxisCalibration *daCalib)
{
    //Get original image
    //..
    int i, N, M;
    QImage img(rootpath()+_PROJECT_NAME+_PATH_DISPLAY_IMAGE);
    M = img.width() * img.height();

    //Creates and fills H
    // ..
    //Creates containers
    int hypW, hypH, hypL;
    QList<double> lstChoises;
    pixel **Hcol;
    int **Hrow;

    lstChoises = getWavesChoised();
    hypW = daCalib->squareUsableW;
    hypH = daCalib->squareUsableH;
    hypL = lstChoises.count();
    N = hypW * hypH * hypL; //Voxels

    //Reserves Memory for H
    //..
    Hcol = (pixel **)malloc(N * sizeof(pixel *));
    for (int n = 0; n < N; n++)
    {
        Hcol[n] = (pixel *)malloc(5 * sizeof(pixel));
    }

    Hrow = (int **)malloc(M * sizeof(int *));
    for (int m = 0; m < M; m++)
    {
        Hrow[m] = (int *)malloc(sizeof(int));
        Hrow[m][0] = 0;
    }

    //It creates H
    //..
    createsHColAndHrow(Hcol, Hrow, &img, daCalib);

    //It creates image to proccess
    //..
    double *g, *gTmp, *f, *fKPlusOne;
    gTmp = (double *)malloc(M * sizeof(double));
    fKPlusOne = (double *)malloc(N * sizeof(double));
    g = serializeImageToProccess(img, sensor); //g
    f = createsF0(Hcol, g, N);                 //f0
    for (i = 0; i < numIterations; i++)
    {
        createsGTmp(gTmp, g, Hrow, f, M); //(Hf)m
        improveF(fKPlusOne, Hcol, f, gTmp, N);
        memcpy(f, fKPlusOne, (N * sizeof(double)));
    }

    //It finishes
    return f;
}


bool generatesHypcube(int numIterations, QString fileName)
{

    // mouseCursorWait();

    int i, l, aux, N;
    double *F, *fRed, *fGreen, *fBlue;
    QList<double> lstChoises;
    int hypW, hypH, hypL;
    lstDoubleAxisCalibration daCalib;
    lstChoises = getWavesChoised();
    funcGetCalibration(&daCalib);
    cout<<daCalib.H<<"\n";
    hypW = daCalib.squareUsableW;
    hypH = daCalib.squareUsableH;
    hypL = lstChoises.count();
    N = hypW * hypH * hypL; //Voxels

    F = (double *)malloc(N * sizeof(double));
    fRed = calculatesF(numIterations, _RED, &daCalib);
    fGreen = calculatesF(numIterations, _GREEN, &daCalib);
    fBlue = calculatesF(numIterations, _BLUE, &daCalib);

    //---------------------------------------------
    //Demosaicing hypercube
    //---------------------------------------------
    if (true)
    {
        if (false)
        {
            fRed = demosaiseF2D(fRed, hypL, hypH, hypW);
            fGreen = demosaiseF2D(fGreen, hypL, hypH, hypW);
            fBlue = demosaiseF2D(fBlue, hypL, hypH, hypW);
        }
        else
        {
            if (SQUARE_BICUBIC_ITERATIONS > 0)
            {
                //Da otra pasada
                for (i = 0; i < SQUARE_BICUBIC_ITERATIONS; i++)
                {
                    fRed = demosaiseF3D(fRed, hypL, hypH, hypW);
                    fGreen = demosaiseF3D(fGreen, hypL, hypH, hypW);
                    fBlue = demosaiseF3D(fBlue, hypL, hypH, hypW);
                }
            }
        }
    }

    //---------------------------------------------
    //Extracting spectral measure
    //---------------------------------------------
    //Get hash to the corresponding sensitivity
    QList<double> Sr;
    QList<double> Sg;
    QList<double> Sb;
    for (l = 0; l < hypL; l++)
    {
        //qDebug() << "Hola1" << l << " of "<< hypL << " size: " << daCalib.Sr.size();
        aux = ((floor(lstChoises.at(l)) - floor(daCalib.minWavelength)) == 0) ? 0 : floor((floor(lstChoises.at(l)) - floor(daCalib.minWavelength)) / (double)daCalib.minSpecRes);
        Sr.append(daCalib.Sr.at(aux));
        Sg.append(daCalib.Sg.at(aux));
        Sb.append(daCalib.Sb.at(aux));
    }
    //qDebug() << "Hola13";
    int j, pixByImage;
    double min, max;
    int minPos, maxPos;
    min = 9999;
    max = -1;
    pixByImage = daCalib.squareUsableW * daCalib.squareUsableH;
    i = 0;
    for (l = 0; l < hypL; l++)
    {
        for (j = 0; j < pixByImage; j++)
        {
            F[i] = (fRed[i] + fGreen[i] + fBlue[i]) / (Sr.at(l) + Sg.at(l) + Sb.at(l));
            if (min > F[i])
            {
                min = F[i];
                minPos = i;
            }
            if (max < F[i])
            {
                max = F[i];
                maxPos = i;
            }
            i++;
        }
    }
    printf("min(%lf,%d) max(%lf,%d)\n", min, minPos, max, maxPos);
    fflush(stdout);

    //---------------------------------------------
    //Save hypercube
    //---------------------------------------------
    //Hypercube Format: Date,W,H,L,l1,...,lL,pix_1_l1,pix_2_l1,...pix_n_l1,pix_1_l2,pix_2_l2,...pix_n_l2,...,pix_1_L,pix_2_L,...pix_n_L
    QString hypercube;
    QDateTime dateTime = QDateTime::currentDateTime();
    hypercube.append(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
    hypercube.append("," + QString::number(daCalib.squareUsableW));
    hypercube.append("," + QString::number(daCalib.squareUsableH));
    hypercube.append("," + QString::number(lstChoises.count()));
    for (l = 0; l < lstChoises.count(); l++)
    {
        hypercube.append("," + QString::number(lstChoises.at(l)));
    }
    N = hypW * hypH * hypL; //Voxels
    for (i = 0; i < N; i++)
    {
        hypercube.append("," + QString::number(F[i]));
    }
    fileName.replace(".hypercube", "");
    saveFile(fileName + ".hypercube", hypercube);

//     mouseCursorReset();

    /*

    if(false)
    {
        double min, max;
        int minPos, maxPos;
        min = 9999;
        max = -1;

        for(int n=0; n<N; n++)
        {
            if(min > F[n])
            {
                min = F[n];
                minPos = n;
            }
            if(max < F[n])
            {
                max = F[n];
                maxPos = n;
            }
           //    printf("F[%d] | %lf\n",n,F[n]);
        }
        printf("min(%lf,%d) max(%lf,%d)\n",min,minPos,max,maxPos);
        fflush(stdout);
    }
    */

    return true;
}


void funcClearDirFolder(QString path)
{
    QDir dir(path);
    if (dir.exists())
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            QFile::remove(info.absoluteFilePath());
        }
    }
    else
    {
        dir.mkdir(path);
    }
}

void extractsHyperCube(QString originFileName)
{
    qDebug() << "inside extractfn";
    //Extracts information about the hypercube
    //..
    QString qstringHypercube;
    QList<QString> hypItems;
    QList<double> waves;
    qstringHypercube = readFileParam(originFileName);
    hypItems = qstringHypercube.split(",");
    QString dateTime;
    int W, H, L, l;
    dateTime = hypItems.at(0);
    hypItems.removeAt(0);
    W = hypItems.at(0).toInt(0);
    hypItems.removeAt(0);
    H = hypItems.at(0).toInt(0);
    hypItems.removeAt(0);
    L = hypItems.at(0).toInt(0);
    hypItems.removeAt(0);
    for (l = 0; l < L; l++)
    {
        waves.append(hypItems.at(0).toDouble(0));
        hypItems.removeAt(0);
    }

    //Generates the images into a tmpImage
    //..
    funcClearDirFolder(rootpath()+_PROJECT_NAME+_PATH_TMP_HYPCUBES);
    QString tmpFileName;
    //QList<QImage> hypercube;
    QImage tmpImg(W, H, QImage::Format_RGB32);
    int tmpVal;
    int col, row;
    double tmp;
    //max = vectorMaxQListQString(hypItems);
    //qDebug() << "To norm max: " << max;

    //Calculate the max cal for each wavelength
    double max[L + 1];
    int i;
    i = 0;
    max[L] = 0;
    for (l = 0; l < L; l++)
    {
        max[l] = 0;
        for (row = 0; row < H; row++)
        {
            for (col = 0; col < W; col++)
            {
                if (hypItems.at(i).toDouble(0) > max[l])
                {
                    max[l] = hypItems.at(i).toDouble(0);
                }
                if (hypItems.at(i).toDouble(0) > max[L])
                {
                    max[L] = hypItems.at(i).toDouble(0);
                }
                i++;
            }
        }
    }

    //Normalize and export image
    //int tmpMax;
    i = 0;
    for (l = 0; l < L; l++)
    {
        //tmpMax = 0;
        //qDebug() << "max[l]: " << max[l];

        for (row = 0; row < H; row++)
        {
            for (col = 0; col < W; col++)
            {
                tmp = hypItems.at(i).toDouble(0);
                if (true) //Normalize
                {
//                    tmpVal  = (tmp<=0.0)?0:round( (tmp/max[l]) * 255.0 );
                    tmpVal = (tmp <= 0.0) ? 0 : round((tmp / max[L]) * 255.0);
                }
                else
                {
                    tmpVal = tmp;
                }
//                printf("(%lf,%d)",tmpVal);
//                tmpImg.setPixelColor(QPoint(col,row),qGray(tmpVal,tmpVal,tmpVal));
                tmpImg.setPixelColor(QPoint(col, row), qRgb(tmpVal, tmpVal, tmpVal));

                //if(tmpVal>tmpMax)
                //{
                //    tmpMax=tmpVal;
                //}

                i++;
            }
        }

        //qDebug() << "tmpMax: " << tmpMax;

        if (SQUARE_BICUBIC_ITERATIONS > 1)
        {
            tmpFileName =rootpath()+_PROJECT_NAME+ _PATH_TMP_HYPCUBES +
                          QString::number(waves.at(l)) +
                          "." +
                          QString::number(SQUARE_BICUBIC_ITERATIONS) +
                          "Pasadas" +
                          ".png";
        }
        else
        {
            tmpFileName = rootpath()+_PROJECT_NAME+_PATH_TMP_HYPCUBES +
                          QString::number(waves.at(l)) +
                          ".png";
        }
        tmpImg.save(tmpFileName);
        tmpImg.fill(Qt::black);
//        hypercube.append(tmpImg);
    }
}


QString timeToQString( unsigned int totMilli )
{
    unsigned int h          = round(totMilli/(1000*60*60));
    totMilli                = totMilli - (h*(1000*60*60));
    unsigned int m          = round(totMilli/(1000*60));
    totMilli                = totMilli - (m*(1000*60));
    unsigned int s          = round(totMilli/1000);
    unsigned int u          = totMilli - (s*1000);
    QString timeElapsed     = "HH:MM:SS:U( ";
    timeElapsed            += (h>9)?QString::number(h)+":":"0"+QString::number(h)+":";
    timeElapsed            += (m>9)?QString::number(m)+":":"0"+QString::number(m)+":";
    timeElapsed            += (s>9)?QString::number(s)+":":"0"+QString::number(s)+":";
    timeElapsed            += QString::number(u);
    timeElapsed            += " )";
    return timeElapsed;
}

void MainWindow::on_Start_Button_clicked()
{
    // QDir dir = QDir::currentPath();
    // dir.cdUp();
    // //QString rootPath = dir.absolutePath();
    // rootPath = dir.absolutePath();
//    rootPath=rootpath();
//    cout << "i am now in erf " <<endl;
//    cout << "i am now in " <<rootPath.toStdString()   << endl;
//    choseWaveToExtract *form = new choseWaveToExtract(this);
//    form->show();
    //----------------------------------------
    // Validate lst of wavelengths selected
    //----------------------------------------
    QList<double> lstChoises;
    lstChoises = getWavesChoised();
    if (lstChoises.size() <= 0)
    {
        funcShowMsg("ERROR", "Please, select wavelengths to extract");
        return (void)false;
    }

    //----------------------------------------
    // Select hypercube destination
    //----------------------------------------
    QString fileName;
    fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Hypercube as..."),
        "./Hypercubes/",
        tr("Documents (*.hypercube)"));
    if (fileName.isEmpty())
    {
        return (void)NULL;
    }

    QElapsedTimer timeStamp;
    timeStamp.start();
    int tmpEMNumIte;
    tmpEMNumIte = readFileParam(rootpath()+_PROJECT_NAME+_PATH_SETTINGS_EM_ITERATIONS).toInt(0);
    qDebug() << "tmpEMNumIte: " << tmpEMNumIte;
    bool isitTrue=generatesHypcube(tmpEMNumIte, fileName);
    if(isitTrue)
//    if (generatesHypcube(tmpEMNumIte, fileName))
    {
       qDebug() << "generation true";
        //Extracts hypercube
       extractsHyperCube(fileName);
        //Show time to extract files
        QString time;
        time = timeToQString(timeStamp.elapsed());
        qDebug() << time;
        //Inform to the user
        funcShowMsg(" ", "Hypercube exported successfully\n\n" + time);
    }
    exit(2);
}
