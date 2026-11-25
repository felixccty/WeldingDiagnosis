#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCharts>
#include "ui_WeldingDiagnosis.h"


class WeldingDiagnosis : public QMainWindow
{
	Q_OBJECT

public:
	QString ico_path = "";
	int mod_id = 1;//1.检测模式，2.学习模式
	int run_status=2, card_status=1, control_status=3;//1.正常运行，2.警告，3和4.告警闪烁
	int model_count = 0;
	int sample_table_id=0;
	QChartView* heat_signal_chart; //红外热信号图，画布声明
	QChartView* visi_signal_chart; //可见光信号图，画布声明

	QAreaSeries* area_series_H;
	QLineSeries* seriesR;
	QLineSeries* seriesV;
	QChart* chart_lineH;
	QChart* chart_lineV;
	QValueAxis* AxisX_H;
	QValueAxis* AxisY_H;
	
	QChartView* diag_signal_chart;
	QChart* chart_diag;
	QVector<QPointF> diag_signal;
	QSplineSeries* seriesD;
	QValueAxis* AxisX_D;
	QValueAxis* AxisY_D;

	QTimer* curr_signal_timer;
	
public:
	WeldingDiagnosis(QWidget *parent = Q_NULLPTR);
	void ico_initial();

public slots:
	void initial_model_table();
	void check_status();
	void learnning_mode_click();
	void diagnosis_mode_click();
	void show_new_model_window();
	void add_new_model();
	void cancel_new_model();
	void show_card_status();
	void setting_click();
	void diagnosisMod_pressed();
	void diagnosisMod_hover();
	void learningMod_pressed();
	void learningMod_hover();
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	void initial_signal_chart(QChartView* heat_signal_chart, QChartView* visi_signal_chart);
	void initial_curr_signal_chart(QChartView* diag_signal_chart);
	void demo();
	void initial_collect_card();
	int connect_to_card();
	void curr_signal_thread();
	void start_collect();
	void stop_collect();
	void learnning_process();
	void start_learn();
	//void start_learn_button();
	void show_learning_process(int learning_heat_signal);
	void end_learn_loop();
	void stop_learn();
	void write_csv(int,QString, QString, QVector<QString>);
	//void write_learning_result(QString, QString, QVector<QString>);
	void write_learning_data(QVector<int>);
	void write_learning_result(QVector<int>, QVector<double>, QVector<double>, QVector<int>, QVector<int>);
	void sleep_10ms(int ms_count);
	void learning_data_analysis();
	void update_new_sample();
	void click_learn_element(QTableWidgetItem* item);
	void send_control_signal();
	void start_learn_action(int);
	void TCP_socket();
	void TCP_server();

private:
	Ui::WeldingDiagnosisClass ui;
};

class thread_obj : public QObject
{
	Q_OBJECT
public slots:
	void learning_thread();

signals:
	void signal_H(int);
};

class thread_ctl : public QObject
{
	Q_OBJECT
public slots:
	void control_thread();

signals:
	void control_signal(int);
};
