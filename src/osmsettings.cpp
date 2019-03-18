#include "osmsettings.h"

OsmSettings::OsmSettings(QObject *parent): QSettings (parent)
{

}

OsmSettings::OsmSettings(const QString &fileName, QSettings::Format format, QObject *parent) : QSettings(fileName,format,parent)
{

}


void OsmSettings::storeSettings()
{
    //TODO: Add all stored properties
    //AppWindow
    QObject * applicationWindow = engine->rootObjects().first();
    if(!applicationWindow)
        return;
    setValue("window/width",applicationWindow->property("width"));
    setValue("window/height",applicationWindow->property("height"));
    setValue("window/x",applicationWindow->property("x"));
    setValue("window/y",applicationWindow->property("y"));
    //SideBar
    QObject * sideBar = engine->rootObjects().first()->property("dataSourceList").value<QObject*>();
    if(!sideBar)
        return;
    QObject * chartComboBox = sideBar->property("chartComboBox").value<QObject*>();
    if(!chartComboBox)
        return;
    setValue("sidebar/chartComboBox.currentIndex",chartComboBox->property("currentIndex").toInt());
    // Charts
    QObject * charts =  engine->rootObjects().first()->property("charts").value<QObject*>();
    if(!charts)
        return;
    QObject * firstChart = charts->property("firstChart").value<QObject*>();
    QObject * secondChart = charts->property("secondChart").value<QObject*>();
    QObject * thirdChart = charts->property("thirdChart").value<QObject*>();
    if(!firstChart || !secondChart || !thirdChart)
        return;
    setValue("charts/first.height",firstChart->property("height"));
    setValue("charts/second.height",secondChart->property("height"));
    setValue("charts/third.height",thirdChart->property("height"));
    //Charts Type
    setValue("charts/first.type",firstChart->property("chartComponent").value<QObject*>()->property("chartTypeCB").value<QObject*>()->property("currentIndex").toInt());
    setValue("charts/second.type",secondChart->property("chartComponent").value<QObject*>()->property("chartTypeCB").value<QObject*>()->property("currentIndex").toInt());
    setValue("charts/third.type",thirdChart->property("chartComponent").value<QObject*>()->property("chartTypeCB").value<QObject*>()->property("currentIndex").toInt());
    this->sync();
}

void OsmSettings::loadSettings()
{
    //TODO: Add all stored properties
    //AppWindow
    QObject * applicationWindow = engine->rootObjects().first();
    if(!applicationWindow)
        return;
    applicationWindow->setProperty("width",this->value("window/width"));
    applicationWindow->setProperty("height",this->value("window/height"));
    applicationWindow->setProperty("x",this->value("window/x"));
    applicationWindow->setProperty("y",this->value("window/y"));
    // SideBar
    QObject * sideBar = engine->rootObjects().first()->property("dataSourceList").value<QObject*>();
    if(!sideBar)
        return;
    QObject * chartComboBox = sideBar->property("chartComboBox").value<QObject*>();
    if(!chartComboBox)
        return;
    chartComboBox->setProperty("currentIndex",this->value("sidebar/chartComboBox.currentIndex"));
    // Charts
    QObject * charts =  engine->rootObjects().first()->property("charts").value<QObject*>();
    if(!charts)
        return;
    QObject * firstChart = charts->property("firstChart").value<QObject*>();
    QObject * secondChart = charts->property("secondChart").value<QObject*>();
    QObject * thirdChart = charts->property("thirdChart").value<QObject*>();
    if(!firstChart || !secondChart || !thirdChart)
        return;
    firstChart->setProperty("height",this->value("charts/first.height"));
    secondChart->setProperty("height",this->value("charts/second.height"));
    thirdChart->setProperty("height",this->value("charts/third.height"));
    //Charts Type
    firstChart->property("chartComponent").value<QObject*>()->property("chartTypeCB").value<QObject*>()->setProperty("currentIndex",this->value("charts/first.type"));
    secondChart->property("chartComponent").value<QObject*>()->property("chartTypeCB").value<QObject*>()->setProperty("currentIndex",this->value("charts/second.type"));
    thirdChart->property("chartComponent").value<QObject*>()->property("chartTypeCB").value<QObject*>()->setProperty("currentIndex",this->value("charts/third.type"));
}
