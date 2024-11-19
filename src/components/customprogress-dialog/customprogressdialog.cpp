#include "customprogressdialog.h"

CustomProgressDialog::CustomProgressDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Loading");

    // Crear el mensaje
    label = new QLabel("Creating project, please wait...", this);
    label->setAlignment(Qt::AlignCenter); // Centrar el texto

    // Crear la barra de progreso
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);                // Barra de progreso indeterminada
    progressBar->setAlignment(Qt::AlignCenter); // Centrar la barra

    // Layout para centrar el contenido
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label, 0, Qt::AlignCenter);
    layout->addWidget(progressBar, 0, Qt::AlignCenter);
    setLayout(layout);

    // Tamaño del diálogo
    setFixedSize(250, 120); // Ajusta el tamaño si es necesario
}
