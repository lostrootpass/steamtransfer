#include "MainWindow.h"

#include <QtWidgets/QtWidgets>

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent)
{
}


MainWindow::~MainWindow()
{
}

void MainWindow::init()
{
	resize(640, 480);
	setWindowTitle(tr("Steam Transfer Manager"));

	QWidget* centralWidget = new QWidget();
	centralWidget->setLayout(new QVBoxLayout);

	setCentralWidget(centralWidget);

	_initMenuBar();
	_setupLayout();

	statusBar()->show();
}

void MainWindow::_initMenuBar()
{
	QLayout* const layout = centralWidget()->layout();
	QMenuBar* menuBar = new QMenuBar();
	layout->addWidget(menuBar);

	//File menu
	QMenu* file = menuBar->addMenu(tr("&File"));
	file->addAction(tr("&Quit"));


	//Tools menu
	QMenu* tools = menuBar->addMenu(tr("&Tools"));
	tools->addAction("&Preferences");
	

	//Help Menu
	QMenu* help = menuBar->addMenu(tr("&Help"));
	QAction* about = help->addAction("About &STM");
	connect(about, &QAction::triggered, this, &MainWindow::showAbout);

	QAction* aboutQt = help->addAction("About &Qt");
	connect(aboutQt, &QAction::triggered, qApp, QApplication::aboutQt);
}

void MainWindow::_setupLayout()
{
	QLayout* const layout = centralWidget()->layout();
	QStandardItemModel* model = new QStandardItemModel();
	QTableView* view = new QTableView();
	view->setModel(model);
	layout->addWidget(view);
	view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	model->setHeaderData(0, Qt::Horizontal, tr("App ID"));
	model->setHeaderData(1, Qt::Horizontal, tr("Name"));
	model->setHeaderData(2, Qt::Horizontal, tr("Current Location"));

	//for(int i = 0; i < 4; i++)
	//{
	//	model->setItem(i, 0, new QStandardItem(tr("245340")));
	//	model->setItem(i, 1, new QStandardItem(tr("Life is Strange")));
	//	model->setItem(i, 2, new QStandardItem(tr("C:\\Program Files (x86)\\Steam\\steamapps")));
	//}
}

void MainWindow::showAbout()
{
	QMessageBox mb(this);
	mb.setTextFormat(Qt::RichText);
	mb.setWindowTitle(tr("About Steam Transfer Manager"));
	mb.setText(tr("<p>Steam Transfer Manager is released under zlib license.</p><p>Source available at <a href='http://github.com/lostrootpass'>http://github.com/lostrootpass</a></p>"));

	mb.exec();
}

#include <gen/moc_MainWindow.cpp>