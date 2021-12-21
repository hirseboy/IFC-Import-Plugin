#ifndef MainWindowH
#define MainWindowH

#include <QMainWindow>

#include <IFCC_IFCReader.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_actionOpen_triggered();

	void on_actionRead_ifc_triggered();

	void on_actionSave_triggered();

private:
	Ui::MainWindow *ui;
	std::unique_ptr<IFCC::IFCReader> m_reader;

	QStringList statistic() const;
};
#endif // MainWindowH