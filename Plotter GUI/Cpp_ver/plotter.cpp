#include <QtWidgets>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialPlotter : public QWidget
{
    Q_OBJECT

public:
    SerialPlotter(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        // Create a Qwt plot
        plot = new QwtPlot(this);
        plot->setCanvasBackground(Qt::white);

        // Create a curve to display data
        curve = new QwtPlotCurve();
        curve->attach(plot);
        curve->setPen(Qt::blue);

        // Layout
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(plot);
        setLayout(layout);

        // Open the serial port
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
            if (info.description().contains("USB")) { // Adjust this according to your device's description
                serialPort.setPortName(info.portName());
                if (serialPort.open(QIODevice::ReadOnly)) {
                    qDebug() << "Serial port opened:" << info.portName();
                    break;
                } else {
                    qDebug() << "Failed to open serial port:" << info.portName();
                }
            }
        }

        // Connect signals and slots
        connect(&serialPort, &QSerialPort::readyRead, this, &SerialPlotter::readData);
    }

    ~SerialPlotter()
    {
        serialPort.close();
    }

private slots:
    void readData()
    {
        if (serialPort.canReadLine()) {
            QByteArray data = serialPort.readLine();
            QString strData = QString::fromLatin1(data).trimmed();
            bool ok;
            double value = strData.toDouble(&ok);
            if (ok) {
                xData.append(xData.size());
                yData.append(value);
                curve->setSamples(xData, yData);
                plot->replot();
            }
        }
    }

private:
    QSerialPort serialPort;
    QVector<double> xData, yData;
    QwtPlot *plot;
    QwtPlotCurve *curve;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SerialPlotter serialPlotter;
    serialPlotter.resize(800, 600);
    serialPlotter.show();

    return app.exec();
}

#include "main.moc"
