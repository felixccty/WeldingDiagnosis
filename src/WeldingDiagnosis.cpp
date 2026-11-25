
#include "WeldingDiagnosis.h"
#include "USB9825.h"
#include <windows.h>
#include <thread>
#include<list>
#include <iostream>
#include<QThread>
#include <fstream>
#include <time.h>
#include <stdio.h>
#include<QTimer.h>
#include <cmath>
#include <QtCharts>
#include <QRandomGenerator>
#include <QtNetwork>

#ifdef WIN32

#pragma execution_character_set("utf-8")

#endif
using namespace std;

#define  MAX_SEGMENT 3
UCHAR  devNum;  //当前打开设备号
USHORT* dataBuff[MAX_SEGMENT];//采集信息缓冲，采用Block环形缓冲方式
BOOL    ADRun = true;//正在采集标志 
HANDLE hDevice; //设备句柄
LONG m_lChcnt = 1;
ULONG   samcnt = 256 * 1 * m_lChcnt;//采集样点数，1024 * 100 * m_lChcnt;
int throw_point = 0;
PUSHORT inBuffer = new USHORT[samcnt];
int min_y_H = 1000, max_y_H = 2000;
extern bool do_analysize = false;
bool finish_analysize = true;
bool start_learn_check = false;

int connect_status;
int learn_axisH_x = 0;
int learn_table_row_count = 0;
int learn_x_size = 400;
QVector<int> curr_vector_H;
QVector <QVector<QVector<int>>> table_vector_H;
int curr_learn_id = 0;
QString curr_learn_name = "";
QVector < QVector<int>> total_H;
QVector < QVector<double>> avg_H;
QVector < QVector<double>> standardD_H;
QVector < QVector<int>> listH_low;
QVector < QVector<int>> listH_high;
//QTimer* timer3;
QSplineSeries* seriesH_low;
QSplineSeries* seriesH_high;
QSplineSeries* seriesH;
bool analyzing = false;
bool scan_pulse = false;
//写csv 向下添加
void WeldingDiagnosis::write_csv(int write_mod,QString csv_path, QString csv_file_name, QVector<QString> write_list) {
	QFile file_save;
	quint64 length = -1;
	QString csvFile = csv_path + "/" + csv_file_name;
	file_save.setFileName(csvFile);
	if (!QDir(csv_path).exists()) {
		QDir().mkdir(csv_path);
	}
	//向下写入
	if (write_mod==1) {
		if (!file_save.open(QIODevice::Append)) {
			qDebug() << "failed to open file.";

		}
		else {
			for (int i = 0; i < write_list.size(); i++) {
				length = file_save.write(write_list[i].toLatin1(), write_list[i].length());
			}
		}
	}
	//全部覆盖重新写
	else if (write_mod == 2) {
		if (!file_save.open(QIODevice::WriteOnly)) {
			qDebug() << "failed to open file.";

		}
		else {
			for (int i = 0; i < write_list.size(); i++) {
				length = file_save.write(write_list[i].toLatin1(), write_list[i].length());
			}
		}
	}

	if (length == -1) {
		qDebug() << "写入文件失败";
	}
	else {
		qDebug() << "写入文件成功";
	}
	if (file_save.isOpen())
	{
		file_save.close();
	}
}

//string转QString
QString str2qstr(const string str)
{
	return QString::fromLocal8Bit(str.data());
}
//读取所有图标路径
QString read_ico_path() {
	string strPath;
	char buffer[MAX_PATH];
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH在WINDEF.h中定义了，等于260  
	memset(exeFullPath, 0, MAX_PATH);
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, exeFullPath, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, exeFullPath, -1, buffer, iLength, NULL, NULL);
	strPath = (string)buffer;
	int pos = strPath.find_last_of('\\', strPath.length());
	strPath = strPath.substr(0, pos);
	QString ico_path_Qstr = str2qstr(strPath);
	return ico_path_Qstr;
}
void WeldingDiagnosis::sleep_10ms(int ms_count) {
	for (int i = 0; i < ms_count; i++) {
		Sleep(10);
		qApp->processEvents();
	}
}
//切换学习和识别界面
void WeldingDiagnosis::learnning_mode_click() {
	ui.widget_learning->show();
	ui.widget_diagnosis->hide();
	ui.pushButton_learningMod->setGeometry(580, 51, 581, 51);
	ui.pushButton_learningMod->setFont(QFont("微软雅黑", 18, QFont::Bold));
	ui.pushButton_diagnosisMod->setGeometry(0, 51, 581, 41);
	ui.pushButton_diagnosisMod->setFont(QFont("微软雅黑", 14));

	ui.pushButton_learningMod->setStyleSheet("QPushButton{background-color:rgb(21, 144, 84);color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(21, 144, 84); color:rgb(255,255,255); }QPushButton:pressed{background-color:rgb(21, 144, 84);}");
	ui.pushButton_diagnosisMod->setStyleSheet("QPushButton{background-color:rgb(25, 102, 255);color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(58, 176, 255); color:rgb(0,0,0); }QPushButton:pressed{background-color:rgb(0, 56, 168);}");
	ui.layer_title->setStyleSheet("background-color: rgb(21, 144, 84);color: rgb(255, 255, 255); ");
	mod_id = 2;
	ui.pushButton_setting->setStyleSheet("QPushButton{background-color:transparent;color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(0,170,0); color:rgb(0,0,0); }QPushButton:pressed{background-color:rgb(0,100,0);}");
	ui.pushButton_minW->setStyleSheet("QPushButton{background-color:transparent;color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(0,170,0); color:rgb(0,0,0); }QPushButton:pressed{background-color:rgb(0,100,0);}");
	qApp->processEvents();
}
void WeldingDiagnosis::diagnosis_mode_click() {
	ui.widget_learning->hide();
	ui.widget_diagnosis->show();
	ui.pushButton_learningMod->setGeometry(580, 51, 581, 41);
	ui.pushButton_learningMod->setFont(QFont("微软雅黑", 14));
	ui.pushButton_diagnosisMod->setGeometry(0, 51, 581, 51);
	ui.pushButton_diagnosisMod->setFont(QFont("微软雅黑", 18, QFont::Bold));
	ui.pushButton_learningMod->setStyleSheet("QPushButton{background-color:rgb(21, 144, 84);color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(0, 179, 0); color:rgb(0,0,0); }QPushButton:pressed{background-color:rgb(0, 100, 0);}");
	ui.pushButton_diagnosisMod->setStyleSheet("QPushButton{background-color:rgb(25, 102, 255);color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(25, 102, 255); color:rgb(255,255,255); }QPushButton:pressed{background-color:rgb(25, 102, 255);}");
	ui.layer_title->setStyleSheet("background-color: rgb(25, 102, 255);color: rgb(255, 255, 255); ");
	ui.pushButton_setting->setStyleSheet("QPushButton{background-color:transparent;color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(58,176,255); color:rgb(0,0,0); }QPushButton:pressed{background-color:rgb(0,56,168);}");
	ui.pushButton_minW->setStyleSheet("QPushButton{background-color:transparent;color:rgb(255,255,255);border:none;}QPushButton:hover{background-color:rgb(58,176,255); color:rgb(0,0,0); }QPushButton:pressed{background-color:rgb(0,56,168);}");
	mod_id = 1;
	qApp->processEvents();
}
//采集卡等硬件状态信息
void WeldingDiagnosis::check_status() {
	if (run_status == 1) {
		ui.pushButton_runStatus->setStyleSheet("QPushButton{background-color: rgb(58, 230, 58);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
	}
	else if (run_status == 2) {
		ui.pushButton_runStatus->setStyleSheet("QPushButton{background-color: rgb(232, 232, 0);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
	}
	else if (run_status == 3) {
		ui.pushButton_runStatus->setStyleSheet("QPushButton{background-color: rgb(255, 54, 54);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
		run_status = 4;
	}
	else if (run_status == 4) {
		ui.pushButton_runStatus->setStyleSheet("QPushButton{background-color: rgb(214, 44, 44);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
		run_status = 3;
	}

	if (card_status == 1) {
		ui.pushButton_cardConnect->setStyleSheet("QPushButton{background-color: rgb(58, 230, 58);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
	}
	else if (card_status == 2) {
		ui.pushButton_cardConnect->setStyleSheet("QPushButton{background-color: rgb(232, 232, 0);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
	}
	else if (card_status == 3) {
		ui.pushButton_cardConnect->setStyleSheet("QPushButton{background-color: rgb(255, 54, 54);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
		card_status = 4;
	}
	else if (card_status == 4) {
		ui.pushButton_cardConnect->setStyleSheet("QPushButton{background-color: rgb(214, 44, 44);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
		card_status = 3;
	}

	if (control_status == 1) {
		ui.pushButton_controlConnect->setStyleSheet("QPushButton{background-color: rgb(58, 230, 58);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
	}
	else if (control_status == 2) {
		ui.pushButton_controlConnect->setStyleSheet("QPushButton{background-color: rgb(232, 232, 0);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
	}
	else if (control_status == 3) {
		ui.pushButton_controlConnect->setStyleSheet("QPushButton{background-color: rgb(255, 54, 54);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
		control_status = 4;
	}
	else if (control_status == 4) {
		ui.pushButton_controlConnect->setStyleSheet("QPushButton{background-color: rgb(214, 44, 44);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
		control_status = 3;
	}
}
//初始化表格
void WeldingDiagnosis::initial_model_table() {
	ui.tableWidget_model->setColumnWidth(0, 120);
	ui.tableWidget_model->setColumnWidth(1, 45);
	ui.tableWidget_model->verticalHeader()->setFixedWidth(20);


	ui.tableWidget_model->horizontalHeader()->setStyleSheet("QHeaderView::section{border:none}");
	ui.tableWidget_model->verticalHeader()->setStyleSheet("QHeaderView::section{border:none}");
	ui.tableWidget_model->setMouseTracking(false); //  设置鼠标追踪
	ui.tableWidget_model->horizontalHeader()->setHighlightSections(false);
	ui.tableWidget_model->verticalHeader()->setHighlightSections(false);
	ui.tableWidget_model->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.tableWidget_model->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	ui.tableWidget_sample->setColumnWidth(0, 80);
	ui.tableWidget_sample->verticalHeader()->setFixedWidth(20);
	ui.tableWidget_sample->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	ui.tableWidget_model_dia->setColumnWidth(0, 130);
	ui.tableWidget_model_dia->setColumnWidth(1, 50);
	ui.tableWidget_model_dia->verticalHeader()->setFixedWidth(20);
	ui.tableWidget_model_dia->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);


	
}
//图标初始化
void WeldingDiagnosis::ico_initial() {
	ico_path = read_ico_path();
	setWindowIcon(QIcon(ico_path + "/ico/logo.png"));
	QIcon icon_set;
	icon_set.addFile(ico_path + "/ico/设置.png");
	ui.pushButton_setting->setIconSize(QSize(20, 20));
	ui.pushButton_setting->setIcon(icon_set);
	QIcon icon_min;
	icon_min.addFile(ico_path + "/ico/最小化.png");
	ui.pushButton_minW->setIconSize(QSize(40, 40));
	ui.pushButton_minW->setIcon(icon_min);
	QIcon icon_close;
	icon_close.addFile(ico_path + "/ico/关闭.png");
	ui.pushButton_closeW->setIconSize(QSize(40, 40));
	ui.pushButton_closeW->setIcon(icon_close);
	QIcon icon_newM;
	icon_newM.addFile(ico_path + "/ico/添加模型.png");
	ui.pushButton_add->setIconSize(QSize(20, 20));
	ui.pushButton_add->setIcon(icon_newM);
	QIcon icon_diaM;
	icon_diaM.addFile(ico_path + "/ico/检测模式.png");
	ui.pushButton_diagnosisMod->setIconSize(QSize(75, 75));
	ui.pushButton_diagnosisMod->setIcon(icon_diaM);
	QIcon icon_learnM;
	icon_learnM.addFile(ico_path + "/ico/学习模式.png");
	ui.pushButton_learningMod->setIconSize(QSize(25, 25));
	ui.pushButton_learningMod->setIcon(icon_learnM);
	QIcon icon_title;
	icon_title.addFile(ico_path + "/ico/焊接.png");
	ui.pushButton_titleLogo->setIconSize(QSize(40, 40));
	ui.pushButton_titleLogo->setIcon(icon_title);

	ui.widget_gray->setGeometry(0, 50, 1161, 751);
	ui.widget_gray->hide();
	ui.widget_basic_set->setGeometry(420, 340, 341, 171);
	ui.widget_basic_set->hide();
	ui.widget_add_mod->setGeometry(420, 340, 341, 171);
	ui.widget_add_mod->hide();

	ui.widget_learning->setGeometry(0, 102, 1160, 707);
	ui.widget_diagnosis->setGeometry(0, 102, 1160, 707);

	ui.widget_card_status->setGeometry(690, 60, 341, 171);
	ui.widget_card_status->hide();
}
//添加新模式按钮
void WeldingDiagnosis::show_new_model_window() {
	ui.widget_gray->show();
	ui.widget_basic_set->hide();
	ui.widget_add_mod->show();

}
//添加新模式设置
void WeldingDiagnosis::add_new_model() {
	ui.widget_gray->hide();
	model_count++;
	ui.tableWidget_model->setRowCount(model_count);
	ui.tableWidget_model->setItem(model_count-1, 0, new QTableWidgetItem(ui.lineEdit_mod_name->text()));
	ui.tableWidget_model->setItem(model_count-1, 1, new QTableWidgetItem(QString::number(ui.spinBox_sample_count->value())));
	ui.tableWidget_model_dia->setRowCount(model_count);
	ui.tableWidget_model_dia->setItem(model_count - 1, 0, new QTableWidgetItem(ui.lineEdit_mod_name->text()));
	ui.tableWidget_model_dia->setItem(model_count - 1, 1, new QTableWidgetItem(QString::number(ui.spinBox_sample_count->value())));


}
//取消添加新模式
void WeldingDiagnosis::cancel_new_model() {
	ui.widget_gray->hide();
}
bool m_bDragging;			// 是否正在拖动
QPoint m_poStartPosition;	// 拖动开始前的鼠标位置
QPoint m_poFramePosition;	// 窗体的原始位置
//窗口可拖动点击
void WeldingDiagnosis::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		QRect rect = ui.layer_title->rect();  //rect是鼠标实现可拖动的区域
		rect.setBottom(rect.top() + 50);
		if (rect.contains(event->pos()))
		{
			m_bDragging = true;
			m_poStartPosition = event->globalPos();
			m_poFramePosition = frameGeometry().topLeft();
		}
	}
	QWidget::mousePressEvent(event);
}
//窗口可拖动移动
void WeldingDiagnosis::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)	//只响应鼠标所见
	{
		if (m_bDragging)
		{
			// delta 相对偏移量, 
			QPoint delta = event->globalPos() - m_poStartPosition;
			// 新位置：窗体原始位置  + 偏移量
			move(m_poFramePosition + delta);
		}
	}
	QWidget::mouseMoveEvent(event);
}
//窗口释放
void WeldingDiagnosis::mouseReleaseEvent(QMouseEvent* event)
{
	m_bDragging = false;
	QWidget::mouseReleaseEvent(event);
}
//模式按钮图标变化
void WeldingDiagnosis::diagnosisMod_pressed() {
	QIcon icon_diaM;
	icon_diaM.addFile(ico_path + "/ico/检测模式闭眼.png");
	ui.pushButton_diagnosisMod->setIconSize(QSize(75, 75));
	ui.pushButton_diagnosisMod->setIcon(icon_diaM);
}
void WeldingDiagnosis::diagnosisMod_hover(){
	QIcon icon_diaM;
	icon_diaM.addFile(ico_path + "/ico/检测模式.png");
	ui.pushButton_diagnosisMod->setIconSize(QSize(75, 75));
	ui.pushButton_diagnosisMod->setIcon(icon_diaM);
}
void WeldingDiagnosis::learningMod_pressed() {
	QIcon icon_learnM;
	icon_learnM.addFile(ico_path + "/ico/学习模式点亮.png");
	ui.pushButton_learningMod->setIconSize(QSize(25, 25));
	ui.pushButton_learningMod->setIcon(icon_learnM);
}
void WeldingDiagnosis::learningMod_hover() {
	QIcon icon_learnM;
	icon_learnM.addFile(ico_path + "/ico/学习模式.png");
	ui.pushButton_learningMod->setIconSize(QSize(25, 25));
	ui.pushButton_learningMod->setIcon(icon_learnM);
}
//系统设置
void WeldingDiagnosis::setting_click(){
	if (ui.widget_basic_set->isHidden()) {
		
		ui.widget_basic_set->show();
		ui.widget_add_mod->hide();
		ui.widget_gray->show();
	}
	else {
		ui.widget_gray->hide();
		ui.widget_basic_set->hide();
	}
}
//鼠标悬浮采集卡设置显示
void WeldingDiagnosis::show_card_status() {
	ui.widget_card_status->show();

}
//初始化信号图表
void WeldingDiagnosis::initial_signal_chart(QChartView* heat_signal_chart,QChartView* visi_signal_chart) {
	heat_signal_chart->setStyleSheet(QString("color:rgb(0,0,0);background-color:rgb(255,255,255)"));
	seriesH = new QSplineSeries();
	seriesH_low = new QSplineSeries();
	seriesH_high = new QSplineSeries();
	area_series_H = new QAreaSeries(seriesH_low, seriesH_high);
	area_series_H->setPen(QPen(QBrush(QColor(21, 144, 84)), 1, Qt::SolidLine));
	//seriesH->setName("红外信号曲线");
	QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
	gradient.setColorAt(0.0, 0x3cc63c);
	gradient.setColorAt(1.0, 0x26f626);
	gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	area_series_H->setBrush(gradient);
	seriesH->setPen(QPen(QBrush(QColor(0, 0, 0)), 1, Qt::SolidLine));
	chart_lineH = new QChart();
	chart_lineH->createDefaultAxes();
	chart_lineH->setTitle("红外信号");
	chart_lineH->addSeries(area_series_H);
	chart_lineH->addSeries(seriesH);
	chart_lineH->setContentsMargins(0, 0, 0, 0);  //设置外边界全部为0
	chart_lineH->setMargins(QMargins(0, 0, 0, 0));//设置内边界全部为0
	chart_lineH->setBackgroundRoundness(0);       //设置背景区域无圆角
	AxisX_H = new QValueAxis();
	AxisX_H->setLabelFormat("%d");
	AxisX_H->setTickCount(10);
	AxisX_H->setLabelsColor(Qt::black);
	AxisX_H->setGridLinePen(QPen(QBrush(QColor(200, 200, 200)), 1, Qt::SolidLine));
	chart_lineH->addAxis(AxisX_H, Qt::AlignBottom);
	AxisY_H = new QValueAxis();
	AxisY_H->setTickCount(5);
	//AxisY_H->setLabelFormat("%d");
	AxisY_H->setTitleText("信号强度");
	//AxisY_H->setShadesVisible(true);
	//AxisY_H->setShadesBrush(QBrush(QColor(220, 220, 220)));
	//AxisY_H->setLabelsColor(Qt::black);
	chart_lineH->addAxis(AxisY_H, Qt::AlignLeft);
	//chart_lineH->setBackgroundVisible(false);
	seriesH->attachAxis(AxisX_H);
	seriesH->attachAxis(AxisY_H);
	area_series_H->attachAxis(AxisX_H);
	area_series_H->attachAxis(AxisY_H);
	chart_lineH->legend()->setVisible(false);
	heat_signal_chart->setChart(chart_lineH);
	heat_signal_chart->setRubberBand(QChartView::RectangleRubberBand);
	heat_signal_chart->setRenderHint(QPainter::Antialiasing);
	//visi_signal_chart->setStyleSheet(QString("color:rgb(0,0,0);background-color:rgb(255,255,255)"));
	chart_lineV = new QChart();
	chart_lineV->createDefaultAxes();
	chart_lineV->setTitle("可见光信号");
	chart_lineV->setContentsMargins(0, 0, 0, 0);  //设置外边界全部为0
	chart_lineV->setMargins(QMargins(0, 0, 0, 0));//设置内边界全部为0
	chart_lineV->setBackgroundRoundness(0);       //设置背景区域无圆角
	//visi_signal_chart->setChart(chart_lineV);
	//visi_signal_chart->setRubberBand(QChartView::RectangleRubberBand);
	//visi_signal_chart->setRenderHint(QPainter::Antialiasing);
	
}
//初始化实时显示信号图
void WeldingDiagnosis::initial_curr_signal_chart(QChartView* diag_signal_chart) {

	diag_signal_chart->setStyleSheet(QString("color:rgb(0,0,0);background-color:rgb(255,255,255);border:none;"));
	seriesD = new QSplineSeries();
	seriesD->setPen(QPen(QBrush(QColor(58, 230, 58)), 1, Qt::SolidLine));
	chart_diag = new QChart();
	chart_diag->setBackgroundVisible(false);
	chart_diag->createDefaultAxes();
	//chart_diag->setTitle("检测红外信号");
	chart_diag->addSeries(seriesD);
	chart_diag->setContentsMargins(0, -10, 0, -20);  //设置外边界全部为0
	chart_diag->setMargins(QMargins(0, 0, 0, 0));//设置内边界全部为0
	chart_diag->setBackgroundRoundness(0);       //设置背景区域无圆角
	AxisX_D = new QValueAxis();
	//AxisX_D->setLabelFormat("%d");
	AxisX_D->setLabelsVisible(false);
	AxisX_D->setTickCount(10);
	AxisX_D->setLabelsColor(Qt::white); 
	//AxisX_D->setGridLinePen(QPen(QBrush(QColor(58, 230, 58)), 0.5, Qt::SolidLine));
	chart_diag->addAxis(AxisX_D, Qt::AlignBottom);
	AxisY_D = new QValueAxis();
	AxisY_D->setTickCount(5);
	AxisY_D->setLabelsColor(Qt::white);
	AxisY_D->setTitleText("实时信号");
	AxisY_D->setTitleBrush(QBrush(QColor(255, 255, 255)));
	chart_diag->addAxis(AxisY_D, Qt::AlignLeft);
	seriesD->attachAxis(AxisX_D);
	seriesD->attachAxis(AxisY_D);
	chart_diag->legend()->setVisible(false);
	diag_signal_chart->setChart(chart_diag);
	diag_signal_chart->setRubberBand(QChartView::RectangleRubberBand);
	diag_signal_chart->setRenderHint(QPainter::Antialiasing);
}
//临时展示
void WeldingDiagnosis::demo() {
	int bar_count = 0;
	bool pass = true;
	int score = 100;
	int x = 0, y = 0;
	int y_low = 0, y_high = 0;
	int x_min = 0, x_max = 400, y_min = 0, y_max = 0;
	int square_edge = 40,sq_count=0;
	bool initial_H = true;
	int sample_table_id = 0;
	int color_flip = 0;
	ui.widget_diagnosis->setStyleSheet("border:none;");
	//AxisX_D->setRange(0, x_max);
	//AxisY_D->setRange(0, 100);
	while (1) {
		QRandomGenerator generator = QRandomGenerator::securelySeeded();
		// 生成一个0到99之间的随机数
		y = generator.bounded(0,20);
		if (sq_count>50) {
			y = y + square_edge;
			if (sq_count >100) {
				sq_count = 0;
			}
		}
		sq_count = sq_count + 1;
		seriesH->append(x, y);
		if (initial_H) {
			y_low = generator.bounded(40, 50);
			y_high = generator.bounded(50, 60);
			//listH_low.append(y);
			//listH_high.append(y);
			seriesH_low->append(x, y);
			seriesH_high->append(x, y);
			//diag_signal.push_back(QPointF(x, y));//信号没填满图
		}
		else {
			//if (y > listH_high[x]) {
			//	listH_high[x] = y;
			//	seriesH_high->replace(x, x, y);
			//}
			//if (y < listH_low[x]) {
			//	listH_low[x] = y;
			//	seriesH_low->replace(x, x, y);
			//}
		}
		AxisX_H->setRange(x_min, x_max);
		AxisY_H->setRange(0, 100);
		//seriesD->replace(diag_signal);
		if (x < x_max) {
			x = x + 1;
		}
		else {
			sq_count = 0;
			x = 0;
			ui.tableWidget_sample->setRowCount(sample_table_id+1);
			ui.tableWidget_sample->setItem(sample_table_id, 0, new QTableWidgetItem("12:59:59"));
			/*
			if (color_flip==0) {
				ui.tableWidget_sample->item(0, sample_table_id)->setBackgroundColor(QColor(173, 200, 154));
				color_flip = 1;
			}
			else {
				color_flip = 0;
				ui.tableWidget_sample->item(0, sample_table_id)->setBackgroundColor(QColor(213, 227, 202));
			}
			*/
			sample_table_id = sample_table_id + 1;
			seriesH->clear();
			//seriesH_low->clear();
			//seriesH_high->clear();
			initial_H = false;
		}
		bar_count = bar_count + 1;
		ui.progressBar->setValue(bar_count);
		if (bar_count >= 100) {
			if (pass) {
				ui.pushButton_result->setStyleSheet("QPushButton{background-color: rgb(58, 230, 58);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
				ui.pushButton_result->setText("合格");
				ui.pushButton_score->setStyleSheet("QPushButton{background-color: transparent;border:0.1px solid gray;border-radius:6px;color:rgb(58, 230, 58);}");
				ui.pushButton_score->setText("100");
				ui.widget_diagnosis->setStyleSheet("border:none;");
				pass = false;
			}
			else {
				ui.pushButton_result->setStyleSheet("QPushButton{background-color: rgb(190, 8, 29);border:0.1px solid gray;border-radius:6px;color: rgb(0, 0, 0);}");
				ui.pushButton_result->setText("不合格");
				ui.pushButton_score->setStyleSheet("QPushButton{background-color: transparent;border:0.1px solid gray;border-radius:6px;color: rgb(255, 54, 54);}");
				ui.pushButton_score->setText("70");
				ui.widget_diagnosis->setStyleSheet("border:5px solid gray;border-color: rgb(255, 0, 0);");
				pass = true;
			}
			bar_count = 0;
		}
		Sleep(10);
		qApp->processEvents();
	}
}
//初始化采集卡设置
void WeldingDiagnosis::initial_collect_card() {
	connect_status=connect_to_card();
	
}
int x_id_d = 0, y_min_d = 2000, y_max_d = 0;
int x_max_d = 200;
int cut_count = 10;
//实时显示信号线程
void WeldingDiagnosis::curr_signal_thread(){
	int buffer_cut = int(samcnt / cut_count);
	USB9825_ReadAD(hDevice, inBuffer, samcnt);
	if (inBuffer[0] > y_max_d) {
		y_max_d = inBuffer[0];
	}
	if (inBuffer[0] < y_min_d) {
		y_min_d = inBuffer[0];
	}
	AxisY_D->setRange(y_min_d, y_max_d);
	if (x_id_d < x_max_d) {
		
		diag_signal.push_back(QPointF(x_id_d, inBuffer[0]));//信号没填满图

		//for (int j = 0; j < 3; ++j) {
		//	diag_signal.push_back(QPointF(x_id_d+j, inBuffer[j* buffer_cut]));//信号没填满图
		//}
		//for (int j = 1; j < samcnt; ++j) {
		//	diag_signal.push_back(QPointF(x_id_d +j, inBuffer[j]));//信号没填满图
		//}
		//int a = size(inBuffer);
		x_id_d = x_id_d+1;
	}
	else {
		for (int i = 1; i < x_max_d; ++i)
		{
			diag_signal[i - 1] = QPointF(diag_signal[i - 1].x(), diag_signal[i].y());
		}
		diag_signal[x_max_d - 1] = QPointF(diag_signal[x_max_d - 1].x(), inBuffer[0]);
	}
	seriesD->replace(diag_signal);
	qApp->processEvents();
	
}
//开始采集
void WeldingDiagnosis::start_collect(){
	if (connect_status == 1) {
		USB9825_PARA_INIT para_init;
		para_init.TriggerMode = TRIG_MODE_CONTINUE;//触发模式 TRIG_MODE_CONTINUE连续采集,TRIG_MODE_POST后触发
		para_init.TriggerSource = TRIG_SRC_INT_RISING;//触发源  TRIG_SRC_EXT_RISING外正沿触发，TRIG_SRC_INT_RISING PWM 正沿触发
		para_init.lSelADClk = ADCLK_INT;//选择采样钟 ADCLK_IN板上时钟 内时钟
		para_init.lChCnt = 1;//通道数
		para_init.ClkDeci = throw_point;//频率因子 //分频因子，0表示1分频 1表示2分频 2表示3分频
		para_init.TriggerLength = 1024;//触发长度,默认1024
		para_init.TriggerDelay = 100; //触发延时，仅延时触发有效，以基准时钟100M为单位
		para_init.TriggerLevel = 2048;//触发门限
		para_init.lADFmt = ADFMT_STBIN;//直接二进制输出

		if (USB9825_InitAD(hDevice, &para_init))
		{
			memset(inBuffer, 0, samcnt * sizeof(USHORT));
			//ui.label_2->setText("初始化成功");
			if (USB9825_ExeSoftTrig(hDevice))
			{
				ui.pushButton_runStatus->setText("软件触发成功");
				qApp->processEvents();
				ADRun = true;
			}
		}
		else {
			ui.pushButton_runStatus->setText("初始化失败");
			qApp->processEvents();
		}

		AxisX_D->setRange(0, x_max_d);
		curr_signal_timer = new QTimer(this);
		curr_signal_timer->setInterval(1);
		connect(curr_signal_timer, SIGNAL(timeout()), this, SLOT(curr_signal_thread()));
		curr_signal_timer->start();

	}
	
}
//停止采集
void WeldingDiagnosis::stop_collect() {
	USB9825_StopAD(hDevice, devNum);
	//curr_signal_timer->stop();
	
}
//连接采集卡
int WeldingDiagnosis::connect_to_card() {
	int i = 0;
	//打开设备
	for (i = 0; i < MAX_SEGMENT; i++)
	{
		dataBuff[i] = NULL;
	}
	hDevice = INVALID_HANDLE_VALUE;
	//依次打开设备号0、1、2...31，如果只有一个卡连接，则设备号0，连接上第二个设备，
	//则设备号是1
	BOOL bOpen = FALSE;
	devNum = 0;
	for (i = 0; i < 32; i++)
	{
		hDevice = USB9825_Link(i);
		qApp->processEvents();
		if (hDevice != INVALID_HANDLE_VALUE) {
			devNum = i;
			//连接成功
			ui.pushButton_cardConnect->setText("连接成功");
			QString devNum_str = QString::number(devNum);
			card_status = 1;
			qApp->processEvents();
			return 1;
			break;
		}
	}
	if (i == 32) {
		//没找到设备
		ui.pushButton_cardConnect->setText("连接失败");
		card_status = 3;
		qApp->processEvents();
		return 0;
	}
}

void WeldingDiagnosis::show_learning_process(int learning_heat_signal) {
	seriesH->append(learn_axisH_x, learning_heat_signal);
	learn_axisH_x = learn_axisH_x + 1;
	curr_vector_H.append(learning_heat_signal);
	if (learn_table_row_count == 0) {
		if (learn_axisH_x < 400) {
			AxisX_H->setRange(0, 400);
		}else{
			AxisX_H->setRange(0, learn_x_size);
		}
	}
	
	if (min_y_H> learning_heat_signal) {
		min_y_H = learning_heat_signal;
	}
	if (max_y_H < learning_heat_signal) {
		max_y_H = learning_heat_signal;
	}
	AxisY_H->setRange(min_y_H-1000, max_y_H+1000);
	qApp->processEvents();
}

//学习线程
void thread_obj::learning_thread() {
	int  first_data = 0;
	while (1) {
		if (start_learn_check) {
			USB9825_ReadAD(hDevice, inBuffer, samcnt);
			first_data = inBuffer[0];
			emit signal_H(first_data);
		}
		Sleep(3);
	}
}
void WeldingDiagnosis::start_learn() {
	if (connect_status == 1) {
		int AD_BASE_FREQ = 40000000;//40M 采样率
		int PWM_Freq = 100;
		int lAllcnt = AD_BASE_FREQ / PWM_Freq;
		USB9825_SetPulGen(hDevice, lAllcnt, lAllcnt / 2, true);
		USB9825_PARA_INIT para_init;
		para_init.TriggerMode = TRIG_MODE_POST;//触发模式 TRIG_MODE_CONTINUE连续采集,TRIG_MODE_POST后触发
		para_init.TriggerSource = TRIG_SRC_INT_RISING;//触发源  TRIG_SRC_EXT_RISING外正沿触发，TRIG_SRC_INT_RISING PWM 正沿触发
		para_init.lSelADClk = ADCLK_INT;//选择采样钟 ADCLK_IN板上时钟 内时钟
		para_init.lChCnt = 1;//通道数
		para_init.ClkDeci = throw_point;//频率因子 //分频因子，0表示1分频 1表示2分频 2表示3分频
		para_init.TriggerLength = 10;//触发长度,默认1024
		para_init.TriggerDelay = 100; //触发延时，仅延时触发有效，以基准时钟100M为单位
		para_init.TriggerLevel = 2048;//触发门限
		para_init.lADFmt = ADFMT_STBIN;//直接二进制输出
		if (USB9825_InitAD(hDevice, &para_init))
		{
			memset(inBuffer, 0, samcnt * sizeof(USHORT));
			ui.pushButton_runStatus->setText("设置成功");
			qApp->processEvents();
			ADRun = true;
			start_learn_check = true;
		}
		else {
			ui.pushButton_runStatus->setText("设置失败");
			qApp->processEvents();
		}
	}
}

void WeldingDiagnosis::end_learn_loop() {
	while (1) {
		if (finish_analysize) {
			break;
		}
		Sleep(1);
	}
	start_learn_check = false;
	USB9825_StopAD(hDevice, devNum);
	do_analysize = true;
	//sleep_10ms(300);
	start_learn();
		//QString save_data = "";
		/*
		if (learn_table_row_count == 0) {
			learn_x_size = temp_vector_H.length();
		}

		for (int write_id = 0; write_id< learn_x_size; write_id++) {
			if (write_id == temp_vector_H.length() - 1) {
				save_data = save_data + QString::number(temp_vector_H[write_id]);
			}
			else {
				save_data = save_data + QString::number(temp_vector_H[write_id]) + ",";
			}
		}
		QVector<QString> write_list;
		write_list.append(save_data+"\n");
		write_csv(read_ico_path()+"/Data", "test.csv", write_list);
		*/
		//learn_table_row_count = learn_table_row_count + 1;
		
}
void WeldingDiagnosis::write_learning_data(QVector<int> write_data ) {
	QString save_data = "";
	int learn_x_size = 0;
	learn_x_size = write_data.length();
	for (int write_id = 0; write_id < learn_x_size; write_id++) {
		if (write_id == learn_x_size - 1) {
			save_data = save_data + QString::number(write_data[write_id]);
		}
		else {
			save_data = save_data + QString::number(write_data[write_id]) + ",";
		}
	}
	QVector<QString> write_list;
	write_list.append(save_data + "\n");
	write_csv(1,read_ico_path() + "/Data", "test.csv", write_list);

}

void WeldingDiagnosis::write_learning_result(QVector<int> write_data_t, QVector<double>write_data_a, QVector<double>write_data_s, QVector<int>write_data_l, QVector<int>write_data_h) {
	QVector<QString> write_list;
	QString save_data = "";
	int learn_x_size = 0;

	learn_x_size = write_data_t.length();
	for (int write_id = 0; write_id < learn_x_size; write_id++) {

		if (write_id == learn_x_size - 1) {
			save_data = save_data + QString::number(write_data_t[write_id]);
		}
		else {
			save_data = save_data + QString::number(write_data_t[write_id]) + ",";
		}
	}
	write_list.append(save_data + "\n");

	save_data = "";
	learn_x_size = write_data_a.length();
	for (int write_id = 0; write_id < learn_x_size; write_id++) {

		if (write_id == learn_x_size - 1) {
			save_data = save_data + QString::number(write_data_a[write_id]);
		}
		else {
			save_data = save_data + QString::number(write_data_a[write_id]) + ",";
		}
	}
	write_list.append(save_data + "\n");

	save_data = "";
	learn_x_size = write_data_s.length();
	for (int write_id = 0; write_id < learn_x_size; write_id++) {

		if (write_id == learn_x_size - 1) {
			save_data = save_data + QString::number(write_data_s[write_id]);
		}
		else {
			save_data = save_data + QString::number(write_data_s[write_id]) + ",";
		}
	}
	write_list.append(save_data + "\n");

	save_data = "";
	learn_x_size = write_data_l.length();
	for (int write_id = 0; write_id < learn_x_size; write_id++) {
		if (write_id == learn_x_size - 1) {
			save_data = save_data + QString::number(write_data_l[write_id]);}
		else {
			save_data = save_data + QString::number(write_data_l[write_id]) + ",";}
	}
	write_list.append(save_data + "\n");

	save_data = "";
	learn_x_size = write_data_h.length();
	for (int write_id = 0; write_id < learn_x_size; write_id++) {
		if (write_id == learn_x_size - 1) {
			save_data = save_data + QString::number(write_data_h[write_id]);
		}
		else {
			save_data = save_data + QString::number(write_data_h[write_id]) + ",";
		}
	}
	write_list.append(save_data + "\n");

	write_csv(1, read_ico_path() + "/Data", "test_result.csv", write_list);

}

void WeldingDiagnosis::learning_data_analysis() {
	if (do_analysize && analyzing==false) {
		analyzing = true;
		//timer3->stop();
		do_analysize = false;
		QVector<int> temp_vector_H(curr_vector_H);
		QVector<QVector<int>> temp_group;
		if (learn_table_row_count == 0) {
			
			temp_group.append(temp_vector_H);
			table_vector_H.append(temp_group);
			//table_vector_H[curr_learn_id].append(temp_vector_H);
			learn_table_row_count = learn_table_row_count + 1;

			total_H.append(temp_vector_H);
			learn_x_size = temp_vector_H.length();

			QVector<double>temp_avg_H;
			QVector<int>temp_listH_low;
			QVector<int>temp_listH_high;
			for (int temp_i = 0; temp_i < learn_axisH_x; temp_i++) {

				temp_avg_H.append(double(temp_vector_H[temp_i]));
				temp_listH_low.append(temp_vector_H[temp_i]);
				temp_listH_high.append(temp_vector_H[temp_i]);
				seriesH_low->append(temp_i, temp_vector_H[temp_i]);
				seriesH_high->append(temp_i, temp_vector_H[temp_i]);
			}

			avg_H.append(temp_avg_H);
			listH_low.append(temp_listH_low);
			listH_high.append(temp_listH_high);


			QVector<double>temp_standardD_H;
			temp_standardD_H.fill(0, temp_avg_H.length());
			standardD_H.append(temp_standardD_H);
			curr_vector_H.clear();
			seriesH->clear();
			learn_axisH_x = 0;
		}
		else {
			temp_vector_H.resize(learn_x_size);
			table_vector_H[curr_learn_id].append(temp_vector_H);
			for (int index_h = 0; index_h < learn_x_size; index_h++) {
				total_H[curr_learn_id][index_h] = total_H[curr_learn_id][index_h] + temp_vector_H[index_h];
				avg_H[curr_learn_id][index_h] = double(total_H[curr_learn_id][index_h] / double(learn_table_row_count));
				double total_SD = 0;
				for (int index_sd = 0; index_sd < learn_table_row_count; index_sd++) {
					total_SD = pow(avg_H[curr_learn_id][index_h] - table_vector_H[curr_learn_id][index_sd][index_h], 2);
				}
				standardD_H[curr_learn_id][index_h] = sqrt(double(total_SD) / double(learn_table_row_count));
				int three_SD_high_H = avg_H[curr_learn_id][index_h] + (standardD_H[curr_learn_id][index_h] * 3);
				int three_SD_low_H = avg_H[curr_learn_id][index_h] - (standardD_H[curr_learn_id][index_h] * 3);
				listH_low[curr_learn_id][index_h] = three_SD_low_H;
				listH_high[curr_learn_id][index_h] = three_SD_high_H;
				
				seriesH_high->replace(index_h, index_h, three_SD_high_H);
				seriesH_low->replace(index_h, index_h, three_SD_low_H);
			}
			learn_table_row_count = learn_table_row_count + 1;
			curr_vector_H.clear();
			seriesH->clear();
			learn_axisH_x = 0;
			//do_analysize = false;
		}
		write_learning_result(total_H[curr_learn_id], avg_H[curr_learn_id], standardD_H[curr_learn_id], listH_low[curr_learn_id], listH_high[curr_learn_id]);
		write_learning_data(temp_vector_H);
		update_new_sample();
		finish_analysize = true;
		analyzing = false;
	}

}

//学习模式进程
void WeldingDiagnosis::learnning_process() {

	thread_obj* thread_showH;
	QThread* thread_H;
	thread_showH = new thread_obj();
	thread_H = new QThread;
	connect(thread_H, SIGNAL(started()), thread_showH, SLOT(learning_thread()));
	connect(thread_showH, SIGNAL(signal_H(int)), this, SLOT(show_learning_process(int)));
	thread_showH->moveToThread(thread_H);
	thread_H->start();//开始线程
	//thread_H->exit();



	thread_ctl* thread_control;
	QThread* thread_C;
	thread_control = new thread_ctl();
	thread_C = new QThread;
	connect(thread_C, SIGNAL(started()), thread_control, SLOT(control_thread()));
	connect(thread_control, SIGNAL(control_signal(int)), this, SLOT(start_learn_action(int)));
	thread_control->moveToThread(thread_C);
	thread_C->start();





	QTimer* analysis_timer = new QTimer(this);
	connect(analysis_timer, SIGNAL(timeout()), this, SLOT(learning_data_analysis()));
	analysis_timer->start(10);

}
void WeldingDiagnosis::stop_learn() {
	//timer3->stop();
	USB9825_StopAD(hDevice, devNum);
	do_analysize = false;
	scan_pulse = false;
}
void WeldingDiagnosis::start_learn_action(int ctl_signal) {

	if (!scan_pulse) {
		start_learn();

		scan_pulse = true;
	}
	else {
		USB9825_StopAD(hDevice, devNum);
		start_learn_check = false;
		scan_pulse = false;
		
		do_analysize = true;
	}
	
	//timer3->start(5000);
	
}

void thread_ctl::control_thread() {
	bool ttl_up = false;
	HANDLE hEvent;
	
	DWORD dwWaitFlag;
	
	while (1) {

		hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, L"FELIX_WELD_SIGNAL_001");
		if (hEvent != NULL) {
			dwWaitFlag = WaitForSingleObject(hEvent, INFINITE);
			emit control_signal(1);
			hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, L"FELIX_WELD_SIGNAL_001");
		}
		
		
		Sleep(1000);
	}
}

void WeldingDiagnosis::send_control_signal() {
	static HANDLE s_hEnventAquire = INVALID_HANDLE_VALUE;
	if (s_hEnventAquire == INVALID_HANDLE_VALUE) {
		s_hEnventAquire = ::CreateEvent(NULL, FALSE, FALSE, L"FELIX_WELD_SIGNAL_001");
	}
	if (s_hEnventAquire != INVALID_HANDLE_VALUE) {
		::SetEvent(s_hEnventAquire);
	}

}

void WeldingDiagnosis::update_new_sample() {
	QTime cur_time = QTime::currentTime();
	QString text_time = cur_time.toString("hh:mm:ss");
	ui.tableWidget_sample->setRowCount(sample_table_id + 1);

	ui.tableWidget_sample->setItem(sample_table_id, 0, new QTableWidgetItem(text_time));
	sample_table_id++;
}
bool click_enable = true;
int last_click_row = 0;
void WeldingDiagnosis::click_learn_element(QTableWidgetItem* item) {

	if (click_enable) {
		click_enable = false;

		int click_row = item->row();
		for (int id = 0; id < 2; id++) {
			ui.tableWidget_model->item(last_click_row, id)->setBackgroundColor(QColor(255, 255, 255));
			qApp->processEvents();
		}
		
		last_click_row = click_row;
		curr_learn_id = click_row;
		for (int id = 0; id < 2; id++) {
			ui.tableWidget_model->item(click_row, id)->setBackgroundColor(QColor(170, 255, 127));
			qApp->processEvents();
		}

		QString label_learn_name = ui.tableWidget_model->item(click_row, 0)->text();
		ui.label_learn_name->setText(label_learn_name);
		curr_learn_name = label_learn_name;
		click_enable = true;
	}

}
void WeldingDiagnosis::TCP_socket() {
	QTcpSocket* socket = new QTcpSocket(this);
	socket->connectToHost("127.0.0.1", 12345);
	if (socket->waitForConnected()) {
		qDebug() << "Connected to server!";

		// 发送数据
		socket->write("Hello, server!");
		socket->waitForBytesWritten();

		// 接收数据
		socket->waitForReadyRead();
		qDebug() << "Received data:" << socket->readAll();

		socket->close();
	}
	
}
void WeldingDiagnosis::TCP_server() {
	QTcpServer* server = new QTcpServer(this);

	if (server->listen(QHostAddress::Any, 12345)) {
		qDebug() << "Server listening on port 12345...";

		connect(server, &QTcpServer::newConnection, [=]() {
			QTcpSocket* clientSocket = server->nextPendingConnection();
			qDebug() << "New client connected!";

			// 接收客户端数据
			connect(clientSocket, &QTcpSocket::readyRead, [=]() {
				
				
				QByteArray buffer = clientSocket->readAll();
				QJsonParseError error;
				QJsonDocument doc = QJsonDocument::fromJson(buffer, &error);  // 解析JSON字符串
				QJsonObject doc_obj = doc.object();
				QStringList stringlist = doc_obj.keys();
				QJsonValue value = doc_obj["Start"];
				//int a = value.toInt();
				QString start_signal = value.toString();
				if (error.error != QJsonParseError::NoError) {
					qDebug() << "Error parsing JSON string: " << error.errorString();
					return;
				}
				qDebug() << "Received data from client:" << value.toString();

				QJsonObject levan{
				{"SendTime","2023 - 10 - 18 11:02 : 35.143"},
				{"SNCode",""},
				{"Result","OK"},
				{"NgCode",""},
				{"Start","01"},
				{"ListIndex",""},
				{"ConfigIndex","1"}
				};

				QJsonArray jsarray{ levan };
				QJsonDocument jsDoc(jsarray);

				if (start_signal == "01") {
					start_learn();
					jsDoc.object().value("Start") = "01";
					QString jsString = QString::fromLatin1(jsDoc.toJson());
					clientSocket->write(jsString.toLatin1());
					clientSocket->waitForBytesWritten();
				}
				else if (start_signal == "02") {
					start_learn();
					jsDoc.object().value("Start") = "02";
					QString jsString = QString::fromLatin1(jsDoc.toJson());
					clientSocket->write(jsString.toLatin1());
					clientSocket->write("Hello, boy");
					clientSocket->waitForBytesWritten();

				}

				});

			// 客户端断开连接
			connect(clientSocket, &QTcpSocket::disconnected, [=]() {
				qDebug() << "Client disconnected!";
				});
			});
	}
	else {
		qDebug() << "Server could not start!";
	}

}
//主页面初始化
WeldingDiagnosis::WeldingDiagnosis(QWidget *parent)
	: QMainWindow(parent)
{
	
	ui.setupUi(this);
	initial_signal_chart(ui.widget_signalHeat, ui.widget_signalVisi);//初始化信号图
	initial_curr_signal_chart(ui.widget_signalDiag);
	ico_initial();
	initial_collect_card();
	initial_model_table();
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setFixedSize(1160, 809);
	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(check_status()));
	timer->start(1000);
	//TCP_socket();
	TCP_server();
	//TCP_socket();
	learnning_process();
	//start_learn();
	//timer3 = new QTimer(this);
	//connect(timer3, SIGNAL(timeout()), this, SLOT(end_learn_loop()));
	//timer3->start(7000);
	ui.widget_diagnosis->setStyleSheet("border:none;");
	connect(ui.pushButton_learningMod, SIGNAL(clicked()), this, SLOT(learnning_mode_click()));
	connect(ui.pushButton_diagnosisMod, SIGNAL(clicked()), this, SLOT(diagnosis_mode_click()));
	connect(ui.pushButton_add, SIGNAL(clicked()), this, SLOT(show_new_model_window()));
	connect(ui.pushButton_save_mod_set, SIGNAL(clicked()), this, SLOT(add_new_model()));
	connect(ui.pushButton_cancel_mod_set, SIGNAL(clicked()), this, SLOT(cancel_new_model()));
	connect(ui.pushButton_closeW, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.pushButton_minW, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui.pushButton_diagnosisMod, SIGNAL(pressed()), this, SLOT(diagnosisMod_pressed()));
	connect(ui.pushButton_diagnosisMod, SIGNAL(released()), this, SLOT(diagnosisMod_hover()));
	connect(ui.pushButton_learningMod, SIGNAL(pressed()), this, SLOT(learningMod_pressed()));
	connect(ui.pushButton_learningMod, SIGNAL(released()), this, SLOT(learningMod_hover()));
	connect(ui.pushButton_setting, SIGNAL(released()), this, SLOT(setting_click()));
	connect(ui.pushButton_cancel_set, SIGNAL(released()), this, SLOT(setting_click()));
	connect(ui.pushButton_cardConnect, SIGNAL(hovered(bool)), this, SLOT(show_card_status()));
	connect(ui.pushButton_startDectict, SIGNAL(clicked()), this, SLOT(start_collect()));
	connect(ui.pushButton_stop, SIGNAL(clicked()), this, SLOT(stop_collect()));
	connect(ui.pushButton_startLearn, SIGNAL(clicked()), this, SLOT(send_control_signal()));
	connect(ui.pushButton_stopLearn, SIGNAL(clicked()), this, SLOT(stop_learn()));
	//connect(ui.tableWidget_model, &QTableWidget::itemClicked, this, &WeldingDiagnosis::click_learn_element);
	connect(ui.tableWidget_model, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(click_learn_element(QTableWidgetItem*)));

}
