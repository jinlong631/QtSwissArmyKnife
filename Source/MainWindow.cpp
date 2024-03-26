﻿/***************************************************************************************************
 * Copyright 2018-2024 x-tools-author(x-tools@outlook.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part of xTools project.
 *
 * xTools is licensed according to the terms in the file LICENCE(GPL V3) in the root of the source 
 * code directory.
 **************************************************************************************************/
#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QButtonGroup>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QJsonParseError>
#include <QLocale>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStatusBar>
#include <QStyleFactory>
#include <QSysInfo>
#include <QTextBrowser>
#include <QToolBar>
#include <QToolButton>
#include <QVariant>

#include "xToolsAssistantFactory.h"
#include "xToolsInterface.h"
#include "xToolsSettings.h"
#include "xToolsToolBoxUi.h"
#include "xToolsUiInterface.h"
#ifdef X_TOOLS_IMPORT_MODULE_CANBUS_STUDIO
#include "xToolsCanBusStudioUi.h"
#endif
#ifdef X_TOOLS_IMPORT_MODULE_MODBUS_STUDIO
#include "xToolsModbusStudioUi.h"
#endif

#ifdef Q_OS_WIN
#include "SystemTrayIcon.h"
#endif

MainWindow::MainWindow(QWidget* parent)
    : xToolsMainWindow(parent)
{
#ifdef Q_OS_WIN
    // Setup system tray icon.
    auto systemTrayIcon = new SystemTrayIcon(this);
    QObject::connect(systemTrayIcon, &SystemTrayIcon::invokeExit, this, &MainWindow::close);
    QObject::connect(systemTrayIcon, &SystemTrayIcon::invokeShowMainWindow, this, &MainWindow::show);
#endif

    QStackedWidget* stackedWidget = new QStackedWidget();
    setCentralWidget(stackedWidget);

#if 0
#ifdef Q_OS_ANDROID
    setWindowFlags(Qt::FramelessWindowHint);
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    setCentralWidget(scrollArea);
    scrollArea->setWidget(mTabWidget);
#endif
#endif

    QString title = QString("xTools");
    title.append(QString(" "));
    title.append(QString("v"));
    title.append(qApp->applicationVersion());
    setWindowTitle(title);
    setWindowIcon(QIcon(":/Resources/Images/Logo.png"));

    initMenuBar();
    initNav();
    initStatusBar();
}

MainWindow::~MainWindow() {}

void MainWindow::initMenuBar()
{
    initFileMenu();
    initToolMenu();
    initOptionMenu();
    initLanguageMenu();
    initLinksMenu();
    initHelpMenu();
}

#ifdef Q_OS_WIN
void MainWindow::closeEvent(QCloseEvent* event)
{
    auto key = m_settingsKey.exitToSystemTray;
    bool ignore = xToolsSettings::instance()->value(key).toBool();
    if (ignore) {
        this->hide();
        event->ignore();
    }
}
#endif

void MainWindow::initFileMenu()
{
    // Tool box
    QMenu* windowMenu = new QMenu(tr("New Window"), this);
    m_fileMenu->addMenu(windowMenu);
    QList<int> toolTypeList = xToolsToolBoxUi::supportedCommunicationTools();
    for (auto& toolType : toolTypeList) {
        const QString name = xToolsToolBoxUi::communicationToolName(toolType);
        QAction* action = new QAction(name, this);
        windowMenu->addAction(action);
        connect(action, &QAction::triggered, this, [=]() {
            xToolsToolBoxUi* w = new xToolsToolBoxUi();
            w->setContentsMargins(9, 9, 9, 9);
            w->setAttribute(Qt::WA_DeleteOnClose, true);
            w->initialize(toolType);
            w->show();
        });
    }

    // Other tools
#ifdef X_TOOLS_IMPORT_MODULE_MODBUS_STUDIO
    QAction* modbusAction = new QAction("Modbus Studio", this);
    connect(modbusAction, &QAction::triggered, this, [=]() {
        xToolsModbusStudioUi* w = new xToolsModbusStudioUi();
        w->setContentsMargins(9, 9, 9, 9);
        w->setAttribute(Qt::WA_DeleteOnClose, true);
        w->resize(1024, 480);
        w->show();
    });
    windowMenu->addAction(modbusAction);
#endif

#ifdef X_TOOLS_IMPORT_MODULE_CANBUS_STUDIO
    QAction* canbusAction = new QAction("CANBus Studio", this);
    connect(canbusAction, &QAction::triggered, this, [=]() {
        xToolsCanBusStudioUi* w = new xToolsCanBusStudioUi();
        w->setContentsMargins(9, 9, 9, 9);
        w->setAttribute(Qt::WA_DeleteOnClose, true);
        w->resize(1024, 480);
        w->show();
    });
    windowMenu->addAction(canbusAction);
#endif

    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);
}

void MainWindow::initToolMenu()
{
    QMenu* toolMenu = new QMenu(tr("&Tools"));
    menuBar()->insertMenu(m_languageMenu->menuAction(), toolMenu);

    for (auto& type : SAKAssistantsFactory::instance()->supportedAssistants()) {
        QString name = SAKAssistantsFactory::instance()->assistantName(type);
        QAction* action = new QAction(name, this);
        QWidget* assistant = SAKAssistantsFactory::instance()->newAssistant(type);

        Q_ASSERT_X(assistant, __FUNCTION__, "A null assistant widget!");

        assistant->hide();
        toolMenu->addAction(action);
        connect(action, &QAction::triggered, this, [=]() {
            if (assistant->isHidden()) {
                assistant->show();
            } else {
                assistant->activateWindow();
            }
        });
    }
}

void MainWindow::initOptionMenu()
{
    QMenu* mainWindowMenu = new QMenu(tr("Main Window"), this);
    QAction* action = new QAction(tr("Exit to Sysytem Tray"), this);
    action->setCheckable(true);
    mainWindowMenu->addAction(action);
    m_optionMenu->addSeparator();
    m_optionMenu->addMenu(mainWindowMenu);

    QVariant v = xToolsSettings::instance()->value(m_settingsKey.exitToSystemTray);
    if (!v.isNull()) {
        bool isExitToSystemTray = v.toBool();
        action->setChecked(isExitToSystemTray);
    }

    connect(action, &QAction::triggered, this, [=]() {
        bool keep = action->isChecked();
        xToolsSettings::instance()->setValue(m_settingsKey.exitToSystemTray, keep);
    });
}

void MainWindow::initLanguageMenu() {}

void MainWindow::initHelpMenu()
{
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(QIcon(":/Resources/Icons/GitHub.svg"),
                          tr("Get Sources from Github"),
                          this,
                          []() { QDesktopServices::openUrl(QUrl(X_TOOLS_GITHUB_REPOSITORY_URL)); });
    m_helpMenu->addAction(QIcon(":/Resources/Icons/Gitee.svg"),
                          tr("Get Sources from Gitee"),
                          this,
                          []() { QDesktopServices::openUrl(QUrl(X_TOOLS_GITEE_REPOSITORY_URL)); });
    m_helpMenu->addSeparator();
#if 0
    m_helpMenu->addAction(tr("About xTools"), this, &MainWindow::aboutSoftware);
#endif
#ifndef X_TOOLS_BUILD_FOR_STORE
#ifdef Q_OS_WIN
    m_helpMenu->addAction(QIcon(":/Resources/Icons/IconBuy.svg"),
                          tr("Buy from Microsoft App Store"),
                          this,
                          []() {
                              QUrl url("https://www.microsoft.com/store/apps/9P29H1NDNKBB");
                              QDesktopServices::openUrl(url);
                          });
#endif
#endif
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(tr("Release History"),
                          this,
                          &MainWindow::showHistory);
    m_helpMenu->addAction(QIcon(":/Resources/Icons/IconQQ.svg"),
                          tr("Join in QQ Group"),
                          this,
                          &MainWindow::showQrCode);
}

void MainWindow::initLinksMenu()
{
    QMenu* linksMenu = new QMenu(tr("&Links"), this);
    menuBar()->insertMenu(m_helpMenu->menuAction(), linksMenu);

    struct Link
    {
        QString name;
        QString url;
        QString iconPath;
    };
    QList<Link> linkList;
    linkList << Link{tr("Qt Official Download"),
                     QString("http://download.qt.io/official_releases/qt"),
                     QString(":/resources/images/Qt.png")}
             << Link{tr("Qt Official Blog"),
                     QString("https://www.qt.io/blog"),
                     QString(":/resources/images/Qt.png")}
             << Link{tr("Qt Official Release"),
                     QString("https://wiki.qt.io/Qt_5.15_Release"),
                     QString(":/resources/images/Qt.png")}
             << Link{QString(""),
                     QString(""),
                     QString("")}
             << Link{tr("Download xTools from Github"),
                     QString("%1/releases").arg(X_TOOLS_GITHUB_REPOSITORY_URL),
                     QString(":/Resources/Icons/GitHub.svg")}
             << Link{tr("Download xTools from Gitee"),
                     QString("%1/releases").arg(X_TOOLS_GITEE_REPOSITORY_URL),
                     QString(":/Resources/Icons/Gitee.svg")}
             << Link{QString(""),
                     QString(""),
                     QString("")}
             << Link{tr("Office Web Site"),
                     QString("https://qsaker.gitee.io/qsak/"),
                     QString(":/Resources/Images/I18n.png")};

    for (auto& var : linkList) {
        if (var.url.isEmpty()) {
            linksMenu->addSeparator();
            continue;
        }

        QAction* action = new QAction(QIcon(var.iconPath), var.name, this);
        action->setObjectName(var.url);
        linksMenu->addAction(action);

        connect(action, &QAction::triggered, this, [=]() {
            QDesktopServices::openUrl(QUrl(sender()->objectName()));
        });
    }
}

void MainWindow::initNav()
{
    QToolBar* tb = new QToolBar(this);
    addToolBar(Qt::LeftToolBarArea, tb);
    tb->setFloatable(false);
    tb->setMovable(false);
    tb->setOrientation(Qt::Vertical);
    tb->setAllowedAreas(Qt::LeftToolBarArea);

    static QButtonGroup btGroup;
    QList<int> types = xToolsToolBoxUi::supportedCommunicationTools();
    for (int i = 0; i < types.count(); i++) {
        int type = types.at(i);
        xToolsToolBoxUi* toolBoxUi = new xToolsToolBoxUi(this);
        toolBoxUi->initialize(type);

        initNav({&btGroup,
                 xToolsUiInterface::cookedIcon(toolBoxUi->windowIcon()),
                 toolBoxUi->windowTitle(),
                 toolBoxUi,
                 tb});
    }

    tb->addSeparator();

    QString path = ":/Resources/Icons/IconModbus.svg";
#ifdef X_TOOLS_IMPORT_MODULE_MODBUS_STUDIO
    xToolsModbusStudioUi* modbus = new xToolsModbusStudioUi(this);
    initNav({&btGroup, xToolsUiInterface::cookedIcon(QIcon(path)), "Modbus Studio", modbus, tb});
#endif
#ifdef X_TOOLS_IMPORT_MODULE_CANBUS_STUDIO
    xToolsCanBusStudioUi* canbus = new xToolsCanBusStudioUi(this);
    path = ":/Resources/Icons/IconCanBus.svg";
    initNav({&btGroup, xToolsUiInterface::cookedIcon(QIcon(path)), "CANBus Studio", canbus, tb});
#endif
    QLabel* lb = new QLabel(" ");
    tb->addWidget(lb);
    lb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

#if 1
    tb->addSeparator();
    const QString key = m_settingsKey.isTextBesideIcon;
    bool isTextBesideIcon = xToolsSettings::instance()->value(key).toBool();
    auto style = isTextBesideIcon ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    QToolButton* tbt = new QToolButton(this);
    path = ":/Resources/Icons/IconListWithIcon.svg";
    tbt->setIcon(xToolsUiInterface::cookedIcon(QIcon(path)));
    tbt->setCheckable(true);
    tbt->setText(" " + tr("Hide Text"));
    tbt->setToolTip(tr("Click to show(hide) nav text"));
    tbt->setAutoRaise(true);
    tbt->setChecked(isTextBesideIcon);
    tbt->setToolButtonStyle(style);
    tbt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    tb->addWidget(tbt);
    auto bg = &btGroup;
    connect(tbt, &QToolButton::clicked, tbt, [=]() {
        auto bts = bg->buttons();
        auto style = tbt->isChecked() ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
        tbt->setToolButtonStyle(style);
        for (auto& bt : bts) {
            auto cookedBt = qobject_cast<QToolButton*>(bt);

            cookedBt->setToolButtonStyle(style);
        }
        xToolsSettings::instance()->setValue(key, tbt->isChecked());
    });
    tb->addSeparator();
#endif
}

void MainWindow::initNav(const NavContext& ctx)
{
    const QString key = m_settingsKey.isTextBesideIcon;
    bool isTextBesideIcon = xToolsSettings::instance()->value(key).toBool();
    auto style = isTextBesideIcon ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    QToolButton* bt = new QToolButton();
    bt->setAutoRaise(true);
    bt->setIcon(ctx.icon);
    bt->setCheckable(true);
    bt->setToolButtonStyle(style);
    bt->setToolTip(ctx.name);
    bt->setText(" " + ctx.name);
    bt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
#if 0
    bt->setIconSize(QSize(32, 32));
#endif
    ctx.bg->addButton(bt);
    ctx.tb->addWidget(bt);

    if (ctx.page->layout()) {
        ctx.page->layout()->setContentsMargins(0, 0, 0, 0);
    }
    auto stackedWidget = qobject_cast<QStackedWidget*>(centralWidget());
    stackedWidget->addWidget(ctx.page);

    int pageCount = ctx.bg->buttons().count();
    QObject::connect(bt, &QToolButton::clicked, bt, [=]() {
        stackedWidget->setCurrentIndex(pageCount - 1);
        xToolsSettings::instance()->setValue(m_settingsKey.pageIndex, pageCount - 1);
    });

    if (xToolsSettings::instance()->value(m_settingsKey.pageIndex).toInt() == (pageCount - 1)) {
        bt->setChecked(true);
        stackedWidget->setCurrentIndex(pageCount - 1);
    }
}

void MainWindow::initStatusBar()
{
    statusBar()->showMessage("Hello world", 10 * 1000);
}

void MainWindow::aboutSoftware()
{
    struct Info
    {
        QString name;
        QString value;
        bool valueIsUrl;
    };

    QString format = QLocale::system().dateFormat();
    format = format + " " + QLocale::system().timeFormat();
    QString dateTimeString = xToolsInterface::buildDateTime(format);
    QList<Info> infoList;
    infoList << Info{tr("Version"), QString(qApp->applicationVersion()), false}
#ifndef SAK_RELEASE_FOR_APP_STORE
             << Info{tr("Edition"), X_TOOL_EDITION, false}
#endif
             << Info{tr("Author"), QString(X_TOOLS_AUTHOR), false}
             << Info{tr("Email"), QString(X_TOOLS_AUTHOR_EMAIL), false}
             << Info{tr("QQ"), QString("QQ:2869470394"), false}
             << Info{tr("QQ Group"), QString("QQ:952218522"), false}
             << Info{tr("Build Time"), dateTimeString, false}
#ifndef SAK_RELEASE_FOR_APP_STORE
             << Info{tr("Gitee Url"),
                     QString("<a href=%1>%1</a>").arg(X_TOOLS_GITEE_REPOSITORY_URL),
                     true}
             << Info{tr("Gitbub Url"),
                     QString("<a href=%1>%1</a>").arg(X_TOOLS_GITHUB_REPOSITORY_URL),
                     true}
#endif
             << Info{tr("Copyright"),
                     tr("Copyright 2018-%1 x-tools-author(x-tools@outlook.com)."
                        " All rights reserved.")
                         .arg(xToolsInterface::buildDateTime("yyyy")),
                     false};

    QDialog dialog(this);
    dialog.setWindowTitle(tr("About QSAK"));

    QGridLayout* gridLayout = new QGridLayout(&dialog);
    int i = 0;
    for (auto& var : infoList) {
        QLabel* nameLabel = new QLabel(QString("<font color=green>%1</font>").arg(var.name),
                                       &dialog);
        QLabel* valueLabel = new QLabel(var.value, &dialog);
        gridLayout->addWidget(nameLabel, i, 0, 1, 1);
        gridLayout->addWidget(valueLabel, i, 1, 1, 1);
        i += 1;

        if (var.valueIsUrl) {
            connect(valueLabel, &QLabel::linkActivated, [](QString url) {
                QDesktopServices::openUrl(QUrl(url));
            });
        }
    }
    dialog.setLayout(gridLayout);
    dialog.setModal(true);
    dialog.show();
    dialog.exec();
}

void MainWindow::clearConfiguration()
{
    xToolsSettings::instance()->setClearSettings(true);
    rebootRequestion();
}

void MainWindow::rebootRequestion()
{
    QString title = tr("Reboot application to effective");
    QString text = tr("Need to reboot, reboot to effective now?");
    QMessageBox::StandardButtons buttons = QMessageBox::Ok | QMessageBox::Cancel;

    int ret = QMessageBox::information(this, title, text, buttons);
    if (ret != QMessageBox::Ok) {
        return;
    }

    if (QProcess::startDetached(QCoreApplication::applicationFilePath())) {
        qApp->closeAllWindows();
        qApp->exit();
    } else {
        QString text = tr("Can not reboot the application, pelase reboot it manually!");
        QMessageBox::warning(this, tr("Reboot Error"), text);
    }
}

void MainWindow::showHistory()
{
    QDialog dialog;
    dialog.setModal(true);
    dialog.setWindowTitle(tr("Release History"));
    dialog.resize(600, 400);

    QTextBrowser* textBrowser = new QTextBrowser(&dialog);
    QFile file(":/Resources/Files/History.txt");
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        textBrowser->setText(QString::fromUtf8(data));
    }

    QHBoxLayout* layout = new QHBoxLayout(&dialog);
    layout->addWidget(textBrowser);
    dialog.setLayout(layout);
    dialog.show();
    dialog.exec();
}

void MainWindow::showQrCode()
{
    QDialog dialog;
    dialog.setWindowTitle(tr("QR Code"));

    struct QrCodeInfo
    {
        QString title;
        QString qrCode;
    };
    QList<QrCodeInfo> qrCodeInfoList;

    qrCodeInfoList << QrCodeInfo{tr("User QQ Group"), QString(":/Resources/Images/QSAKQQ.jpg")}
                   << QrCodeInfo{tr("Qt QQ Group"), QString(":/Resources/Images/QtQQ.jpg")};

    QTabWidget* tabWidget = new QTabWidget(&dialog);
    for (auto& var : qrCodeInfoList) {
        QLabel* label = new QLabel(tabWidget);
        label->setPixmap(QPixmap::fromImage(QImage(var.qrCode)));
        tabWidget->addTab(label, var.title);
    }

    QHBoxLayout* layout = new QHBoxLayout(&dialog);
    layout->addWidget(tabWidget);
    dialog.setLayout(layout);
    dialog.setModal(true);
    dialog.show();
    dialog.exec();
}

void MainWindow::showDonation()
{
    QDialog dialog(this);
    dialog.setModal(true);
    QHBoxLayout* hBoxLayout = new QHBoxLayout(&dialog);
    QString image = ":/resources/images/WeChat.jpg";
    QLabel* label = new QLabel(&dialog);
    QPixmap pixMap = QPixmap::fromImage(QImage(image));
    label->setPixmap(pixMap.scaledToHeight(400, Qt::SmoothTransformation));
    hBoxLayout->addWidget(label);
    dialog.layout()->addWidget(label);
    dialog.show();
    dialog.exec();
}

void MainWindow::createQtConf()
{
    QString fileName = qtConfFileName();
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        QString message = tr("Can not open file(%1): %2").arg(fileName, file.errorString());
        qWarning() << qPrintable(message);
        return;
    }

    QTextStream out(&file);
    out << "[Platforms]\nWindowsArguments = dpiawareness=0\n";
    file.close();
    qInfo() << "Create Qt configuration file successfully:" << qPrintable(fileName);
}
