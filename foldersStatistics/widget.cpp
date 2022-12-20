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
    ui->label->clear();
    ui->label_4->clear();
    ui->label_5->clear();
    ui->label_6->clear();
    ui->label_8->clear();
    ui->label_2->setText("Общий размер: ");
    ui->label_3->setText("Количество файлов в директории: ");
    ui->label_7->setText("Количество подкаталогов: ");
    ui->label_6->setText("Текущий каталог: ");
}

void Widget::on_treeView_clicked(const QModelIndex &index)
{
    detection_cur_dir(index);
}


void Widget::on_treeView_expanded(const QModelIndex &index)
{
    detection_cur_dir(index);
}

// Запуск
void Widget::detection_cur_dir(const QModelIndex &index)
{
    QString strPath = dirModel->fileInfo(index).absoluteFilePath();
    QString currentDir = dirModel->fileName(index);
    ui->label_5->setText(currentDir);

    QFuture<QList<QString>> rawData = QtConcurrent::run(start_scan, strPath);
    QList data = rawData.result();

    ui->label_8->setText(data.at(0));
    ui->label->setText(data.at(1));
    ui->label_4->setText(data.at(2));
    ui->textEdit->setText(data.at(3));
}

QList<QString> Widget::start_scan(QString strPath)
{
    QList<QString> result;
    quint64 amountOfDirs =  find_amount_of_dirs(strPath);
    quint64 amountOfFiles = find_amount_of_files(strPath);
    QString totalSize = calc_total_size(strPath);
    QString avrgSize = calc_avrg_size(strPath);

    result.append(QString::number(amountOfDirs));
    result.append(QString::number(amountOfFiles));
    result.append(totalSize);
    result.append(avrgSize);

    return result;

}

quint64 Widget::find_amount_of_dirs(QString strPath)
{
    QDir dir = QDir(strPath);
    dir.setFilter(QDir::AllDirs | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList dirsList = dir.entryInfoList();
    return dirsList.size();
}

quint64 Widget::find_amount_of_files(QString strPath)
{
    QDir dir = QDir(strPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList filesList = dir.entryInfoList();
    return filesList.size();
}

QString Widget::calc_total_size(QString strPath)
{
    QDir dir = QDir(strPath);
    quint64 totalSize = 0;

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList filesList = dir.entryInfoList();
    if (filesList.size() == 0)
        return "0";

    for (int i = 0; i < filesList.size(); ++i) {
        QFileInfo fileInfo = filesList.at(i);
        totalSize += fileInfo.size();
    }
    return size_conversion(totalSize);
}

QString Widget::calc_avrg_size(QString strPath)
{
    QDir dir = QDir(strPath);
    bool isUnique;
    QString avrSize;
    QList <QString> suffixes;

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Type);
    QFileInfoList filesList = dir.entryInfoList();

    if (filesList.size() == 0)
        return "";

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


