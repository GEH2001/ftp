# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'mainwindow.ui'
##
## Created by: Qt User Interface Compiler version 6.6.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QAction, QBrush, QColor, QConicalGradient,
    QCursor, QFont, QFontDatabase, QGradient,
    QIcon, QImage, QKeySequence, QLinearGradient,
    QPainter, QPalette, QPixmap, QRadialGradient,
    QTransform)
from PySide6.QtWidgets import (QApplication, QHeaderView, QLabel, QLineEdit,
    QMainWindow, QMenuBar, QProgressBar, QPushButton,
    QSizePolicy, QStatusBar, QTableWidget, QTableWidgetItem,
    QTextEdit, QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(800, 600)
        sizePolicy = QSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(MainWindow.sizePolicy().hasHeightForWidth())
        MainWindow.setSizePolicy(sizePolicy)
        self.action = QAction(MainWindow)
        self.action.setObjectName(u"action")
        self.action_2 = QAction(MainWindow)
        self.action_2.setObjectName(u"action_2")
        self.action_4 = QAction(MainWindow)
        self.action_4.setObjectName(u"action_4")
        self.action_5 = QAction(MainWindow)
        self.action_5.setObjectName(u"action_5")
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.lineEditHost = QLineEdit(self.centralwidget)
        self.lineEditHost.setObjectName(u"lineEditHost")
        self.lineEditHost.setGeometry(QRect(60, 20, 91, 20))
        self.lineEditUser = QLineEdit(self.centralwidget)
        self.lineEditUser.setObjectName(u"lineEditUser")
        self.lineEditUser.setGeometry(QRect(200, 20, 81, 20))
        self.lineEditPass = QLineEdit(self.centralwidget)
        self.lineEditPass.setObjectName(u"lineEditPass")
        self.lineEditPass.setGeometry(QRect(320, 20, 81, 20))
        self.lineEditPort = QLineEdit(self.centralwidget)
        self.lineEditPort.setObjectName(u"lineEditPort")
        self.lineEditPort.setGeometry(QRect(440, 20, 41, 20))
        self.label = QLabel(self.centralwidget)
        self.label.setObjectName(u"label")
        self.label.setGeometry(QRect(20, 20, 31, 16))
        self.label_2 = QLabel(self.centralwidget)
        self.label_2.setObjectName(u"label_2")
        self.label_2.setGeometry(QRect(160, 20, 41, 16))
        self.label_3 = QLabel(self.centralwidget)
        self.label_3.setObjectName(u"label_3")
        self.label_3.setGeometry(QRect(290, 20, 31, 16))
        self.label_4 = QLabel(self.centralwidget)
        self.label_4.setObjectName(u"label_4")
        self.label_4.setGeometry(QRect(410, 20, 31, 16))
        self.connBtn = QPushButton(self.centralwidget)
        self.connBtn.setObjectName(u"connBtn")
        self.connBtn.setGeometry(QRect(510, 20, 75, 24))
        self.textEdit = QTextEdit(self.centralwidget)
        self.textEdit.setObjectName(u"textEdit")
        self.textEdit.setGeometry(QRect(20, 50, 731, 121))
        self.textEdit.setReadOnly(True)
        self.label_5 = QLabel(self.centralwidget)
        self.label_5.setObjectName(u"label_5")
        self.label_5.setGeometry(QRect(20, 180, 51, 20))
        self.lineEditRemote = QLineEdit(self.centralwidget)
        self.lineEditRemote.setObjectName(u"lineEditRemote")
        self.lineEditRemote.setGeometry(QRect(80, 180, 391, 20))
        self.lineEditRemote.setReadOnly(True)
        self.progressBar = QProgressBar(self.centralwidget)
        self.progressBar.setObjectName(u"progressBar")
        self.progressBar.setGeometry(QRect(150, 490, 241, 16))
        self.progressBar.setValue(0)
        self.stopBtn = QPushButton(self.centralwidget)
        self.stopBtn.setObjectName(u"stopBtn")
        self.stopBtn.setGeometry(QRect(150, 520, 75, 24))
        self.restartBtn = QPushButton(self.centralwidget)
        self.restartBtn.setObjectName(u"restartBtn")
        self.restartBtn.setGeometry(QRect(280, 520, 75, 24))
        self.curTransLabel = QLabel(self.centralwidget)
        self.curTransLabel.setObjectName(u"curTransLabel")
        self.curTransLabel.setGeometry(QRect(60, 460, 81, 16))
        self.curTransLabel.setAlignment(Qt.AlignRight|Qt.AlignTrailing|Qt.AlignVCenter)
        self.curTransLineEdit = QLineEdit(self.centralwidget)
        self.curTransLineEdit.setObjectName(u"curTransLineEdit")
        self.curTransLineEdit.setGeometry(QRect(150, 460, 201, 20))
        sizePolicy1 = QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        sizePolicy1.setHorizontalStretch(1)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.curTransLineEdit.sizePolicy().hasHeightForWidth())
        self.curTransLineEdit.setSizePolicy(sizePolicy1)
        self.curTransLineEdit.setReadOnly(True)
        self.disConnBtn = QPushButton(self.centralwidget)
        self.disConnBtn.setObjectName(u"disConnBtn")
        self.disConnBtn.setGeometry(QRect(600, 20, 75, 24))
        self.tableWidgetRemote = QTableWidget(self.centralwidget)
        self.tableWidgetRemote.setObjectName(u"tableWidgetRemote")
        self.tableWidgetRemote.setGeometry(QRect(20, 210, 731, 231))
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 800, 22))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName(u"statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"MainWindow", None))
        self.action.setText(QCoreApplication.translate("MainWindow", u"\u65ad\u5f00\u8fde\u63a5", None))
        self.action_2.setText(QCoreApplication.translate("MainWindow", u"\u91cd\u65b0\u8fde\u63a5", None))
        self.action_4.setText(QCoreApplication.translate("MainWindow", u"\u505c\u6b62\u4f20\u8f93", None))
        self.action_5.setText(QCoreApplication.translate("MainWindow", u"\u6062\u590d\u4f20\u8f93", None))
        self.lineEditHost.setText(QCoreApplication.translate("MainWindow", u"166.111.83.113", None))
        self.lineEditUser.setText(QCoreApplication.translate("MainWindow", u"thss", None))
        self.lineEditPass.setText(QCoreApplication.translate("MainWindow", u"thss2023", None))
        self.lineEditPort.setText(QCoreApplication.translate("MainWindow", u"21", None))
        self.label.setText(QCoreApplication.translate("MainWindow", u"\u4e3b\u673a:", None))
        self.label_2.setText(QCoreApplication.translate("MainWindow", u"\u7528\u6237\u540d:", None))
        self.label_3.setText(QCoreApplication.translate("MainWindow", u"\u5bc6\u7801:", None))
        self.label_4.setText(QCoreApplication.translate("MainWindow", u"\u7aef\u53e3:", None))
        self.connBtn.setText(QCoreApplication.translate("MainWindow", u"\u8fde\u63a5", None))
        self.label_5.setText(QCoreApplication.translate("MainWindow", u"\u8fdc\u7a0b\u7ad9\u70b9:", None))
        self.stopBtn.setText(QCoreApplication.translate("MainWindow", u"\u505c\u6b62\u4f20\u8f93", None))
        self.restartBtn.setText(QCoreApplication.translate("MainWindow", u"\u6062\u590d\u4f20\u8f93", None))
        self.curTransLabel.setText(QCoreApplication.translate("MainWindow", u"\u4f20\u8f93\u6587\u4ef6:", None))
        self.disConnBtn.setText(QCoreApplication.translate("MainWindow", u"\u65ad\u5f00\u8fde\u63a5", None))
    # retranslateUi

