#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui_preparation();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::ui_preparation()
{
    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    dirModel->setRootPath(dirModel->rootPath());

    ui->treeView->setModel(dirModel);
    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setColumnHidden(2, true);
    ui->treeView->setColumnHidden(3, true);
    ui->label_6->setText("Текущий каталог: ");
}

void Widget::on_treeView_clicked(const QModelIndex &index)
{
    dirPath = dirModel->fileInfo(index).absoluteFilePath();
    currentDir = dirModel->fileName(index);
    ui->label_5->setText(currentDir);
}


void Widget::on_treeView_expanded(const QModelIndex &index)
{
    dirPath = dirModel->fileInfo(index).absoluteFilePath();
    currentDir = dirModel->fileName(index);
    ui->label_5->setText(currentDir);
}

void Widget::on_pushButton_clicked()
{
    if(backgroundThread.isRunning())
    {
        backgroundThread.end();
    }
    detection_cur_dir(dirPath);
}

// Запуск
void Widget::detection_cur_dir(QString dirPath)
{
    ui->label_9->setText("Подсчет статистики в: ");
    ui->label_10->setText(currentDir);
    ui->label_2->clear();
    ui->label_3->clear();
    ui->label_7->clear();
    ui->label_8->clear();
    ui->label->clear();
    ui->label_4->clear();
    ui->textEdit->clear();

    backgroundThread = QtConcurrent::run(start_scan, dirPath);

    // Лучшего решения не нашел((, надо будет почитать доки QtConcurrent
    //      ,чтобы хотя бы подчищать лишние потоки
    while(!backgroundThread.isFinished())
    {
        qApp->processEvents();
    }
    //
    QList data = backgroundThread.result();

    ui->label_2->setText("Общий размер: ");
    ui->label_3->setText("Количество файлов в директории: ");
    ui->label_7->setText("Количество подкаталогов: ");

    ui->label_8->setText(data.at(0));
    ui->label->setText(data.at(1));
    ui->label_4->setText(data.at(2));
    ui->textEdit->setText(data.at(3));
}

QList<QString> Widget::start_scan(QString dirPath)
{

    QList<QString> result;
    quint64 amountOfDirs =  find_amount_of_dirs(dirPath);
    quint64 amountOfFiles = find_amount_of_files(dirPath);
    QString totalSize = calc_total_size(dirPath);
    QString avrgSize = calc_avrg_size(dirPath);

    result.append(QString::number(amountOfDirs));
    result.append(QString::number(amountOfFiles));
    result.append(totalSize);
    result.append(avrgSize);

    return result;

}

quint64 Widget::find_amount_of_dirs(QString dirPath)
{
    QDir dir = QDir(dirPath);
    dir.setFilter(QDir::AllDirs | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList dirsList = dir.entryInfoList();
    return dirsList.size();
}

quint64 Widget::find_amount_of_files(QString dirPath)
{
    QFileInfoList filesList;
    QDirIterator dir(dirPath,QDir::Files, QDirIterator::Subdirectories);
    while (dir.hasNext()) {
        QFileInfo fileInfo = dir.nextFileInfo();
        filesList.append(fileInfo);
    }
    return filesList.size();
}

QString Widget::calc_total_size(QString dirPath)
{
    quint64 totalSize = 0;
    QFileInfoList filesList;
    QDirIterator dir(dirPath,QDir::Files, QDirIterator::Subdirectories);

    if (!dir.hasNext())
        return "0";

    while (dir.hasNext()) {
        QFileInfo fileInfo = dir.nextFileInfo();
        totalSize += fileInfo.size();
    }

    return size_conversion(totalSize);
}

QString Widget::calc_avrg_size(QString dirPath)
{
    bool isUnique;
    QString avrSize;
    QList <QString> suffixes;

    QFileInfoList filesList;
    QDirIterator dir(dirPath,QDir::Files, QDirIterator::Subdirectories);

    if (!dir.hasNext())
        return "";

    //составление массива файлов
    while (dir.hasNext()) {
        QFileInfo fileInfo = dir.nextFileInfo();
        filesList.append(fileInfo);
    }

    // составление массива расширений из тех, что есть в данном каталоге
    for (int i = 0; i < filesList.size(); ++i) {
        isUnique = true;
        for (int j = i+1; j < filesList.size(); ++j) {
            if(filesList.at(i).suffix() == filesList.at(j).suffix() ){
                isUnique = false;
            }
        }
        if(isUnique)
            suffixes.append(filesList.at(i).suffix());
    }

    // подсчет среднего размера для каждой группы файлов
    for (int i = 0; i < suffixes.size(); ++i) {
    quint64 tempValue = 0;
    quint64 filesCount = 0;
        for (int j = 0; j < filesList.size(); ++j) {
            if(suffixes.at(i)==filesList.at(j).suffix())
            {
                tempValue += filesList.at(j).size();
                filesCount++;
            }
        }
        avrSize += "Средний размер расширения " + suffixes.at(i) + ": " + size_conversion(tempValue/filesCount) + "." + '\n';
    }
    return avrSize;
}

QString Widget::size_conversion(quint64 fileSize)
{
    if(fileSize == 0)
        return 0;
    QMap<int, QString> mapUnit;
    mapUnit[0] = "Байт";
    mapUnit[1] = "Кб";
    mapUnit[2] = "Мб";
    mapUnit[3] = "Гб";
    mapUnit[4] = "Тб";

    // Переменная si отвечает за систему СИ (Байт -> Кб -> Мб и т.д.)
    int si = 0;
    qreal rawSize = static_cast<qreal>(fileSize);

    while(rawSize>=1024)
    {
        si++;
        rawSize = rawSize/1024;
    }

    QString size = QString::number(round(rawSize*10)/10) + " " + mapUnit[si];
    si = 0;
    return size;
}
