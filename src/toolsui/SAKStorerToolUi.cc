/******************************************************************************
 * Copyright 2023 Qsaker(wuuhaii@outlook.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 *****************************************************************************/
#include <QFileDialog>
#include "SAKStorerTool.hh"
#include "SAKStorerToolUi.hh"
#include "ui_SAKStorerToolUi.h"

SAKStorerToolUi::SAKStorerToolUi(QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::SAKStorerToolUi)
{
    ui->setupUi(this);

    connect(ui->pushButtonSelectFile, &QPushButton::clicked,
            this, &SAKStorerToolUi::onPushButtonSelectFileClicked);
}

SAKStorerToolUi::~SAKStorerToolUi()
{
    delete ui;
}

void SAKStorerToolUi::setupStorer(SAKStorerTool *tool)
{
    if (!tool) {
        return;
    }

    if (mTool) {
        disconnect(ui->checkBoxEnable, &QCheckBox::clicked, mTool, nullptr);
        disconnect(ui->checkBoxDate, &QCheckBox::clicked, mTool, nullptr);
        disconnect(ui->checkBoxTime, &QCheckBox::clicked, mTool, nullptr);
        disconnect(ui->checkBoxMs, &QCheckBox::clicked, mTool, nullptr);
    }

    mTool = tool;
    mTool->setProperty("enable", ui->checkBoxEnable->isChecked());
    mTool->setSaveDate(ui->checkBoxDate->isChecked());
    mTool->setSaveTime(ui->checkBoxTime->isChecked());
    mTool->setSaveMs(ui->checkBoxMs->isChecked());

    connect(ui->checkBoxEnable, &QCheckBox::clicked, this, [=](){
        mTool->setProperty("enable", ui->checkBoxEnable->isChecked());
    });
    connect(ui->checkBoxDate, &QCheckBox::clicked, this, [=](){
        mTool->setSaveDate(ui->checkBoxDate->isChecked());
    });
    connect(ui->checkBoxDate, &QCheckBox::clicked, this, [=](){
        mTool->setSaveTime(ui->checkBoxTime->isChecked());
    });
    connect(ui->checkBoxMs, &QCheckBox::clicked, this, [=](){
        mTool->setSaveMs(ui->checkBoxMs->isChecked());
    });
}

void SAKStorerToolUi::onPushButtonSelectFileClicked()
{
    auto str = QFileDialog::getOpenFileName(this, tr("Save file"),
                                            ".", tr("txt (*.txt); all (*)"));
    qDebug() << str;
}
