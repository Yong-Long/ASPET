# -*- coding: utf-8 -*-
from PyQt5 import QtCore, QtGui, QtWidgets
import sys
import os


class Ui_FBPcfg (object):
    
    def setupUi (self, FBPconifg):
        FBPconifg.setObjectName("FBPconifg")
        FBPconifg.setGeometry(600, 400, 320, 120)
        
        self.layoutWidget = QtWidgets.QWidget(FBPconifg)
        self.layoutWidget.setGeometry(QtCore.QRect(10, 10, 280, 120))
        self.layoutWidget.setObjectName("layoutWidget")
        
        self.formLayout = QtWidgets.QFormLayout(self.layoutWidget)
        self.formLayout.setContentsMargins(0, 0, 0, 0)
        self.formLayout.setSpacing(5)
        self.formLayout.setObjectName("formLayout")
        
        # Title
        self.label_ttl = QtWidgets.QLabel(self.layoutWidget)
        self.label_ttl.setAutoFillBackground(True)
        self.label_ttl.setScaledContents(False)
        self.label_ttl.setAlignment(QtCore.Qt.AlignCenter)
        self.label_ttl.setWordWrap(False)
        self.label_ttl.setOpenExternalLinks(False)
        self.label_ttl.setObjectName("label_ttl")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.SpanningRole, self.label_ttl)
        
        # Source bin file
        self.label_bin = QtWidgets.QLabel(self.layoutWidget)
        self.label_bin.setObjectName("label_bin")
        self.label_bin.setMinimumSize(50, 25)
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.LabelRole, self.label_bin)
        
        self.comboBox_bin = QtWidgets.QComboBox(self.layoutWidget)
        self.comboBox_bin.setObjectName("comboBox_bin")
        self.comboBox_bin.setMinimumSize(200, 25)
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.FieldRole, self.comboBox_bin)

        # List of source bin files
        self.list_bin = [os.path.splitext(f)[0] for f in os.listdir('data/src/tmp/') if f.endswith('.bin')]
        self.comboBox_bin.addItems(self.list_bin)
        self.comboBox_bin.setCurrentIndex(0)
        
        # Ok|cancel
        self.buttonBox = QtWidgets.QDialogButtonBox(self.layoutWidget)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setCenterButtons(True)
        self.buttonBox.setObjectName("buttonBox")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.SpanningRole, self.buttonBox)

        self.retranslateUi(FBPconifg)
        self.buttonBox.accepted.connect(FBPconifg.accept)
        self.buttonBox.rejected.connect(FBPconifg.reject)
        QtCore.QMetaObject.connectSlotsByName(FBPconifg)

    def retranslateUi (self, FBPconifg):
        _translate = QtCore.QCoreApplication.translate
        FBPconifg.setWindowTitle(_translate("FBPconifg", "FBP config"))
        self.label_ttl.setText(_translate("FBPconifg", "Config of FBP"))
        self.label_bin.setText(_translate("Visconfig", "Source bin"))


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    FBPconifg = QtWidgets.QDialog()

    ui = Ui_FBPcfg()
    ui.setupUi(FBPconifg)

    FBPconifg.show()
    sys.exit(app.exec_())