#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtGui>
#include <QtCore>
#include <QList>
#include <QDebug>
#include <QtConcurrent>
#include <QFuture>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_treeView_expanded(const QModelIndex &index);
    void on_treeView_clicked(const QModelIndex &index);

    void on_pushButton_clicked();

public slots:

signals:

private:
    Ui::Widget *ui;
    QFileSystemModel *dirModel;
    QString dirPath;
    QString currentDir;
    QFuture<QList<QString>> backgroundThread;

    void ui_preparation();
    void detection_cur_dir(QString strPath);

    static quint64 find_amount_of_dirs(QString strPath);
    static quint64 find_amount_of_files(QString strPath);
    static QString calc_total_size(QString strPath);
    static QString calc_avrg_size(QString strPath);
    static QString size_conversion(quint64 fileSize);
    static QList<QString> start_scan(QString strPath);
};
#endif // WIDGET_H
