from PyQt5.QtCore import QSize, Qt
from PyQt5.QtWidgets import *
from PyQt5 import QtGui
from PyQt5.QtGui import *

from utils.function.show_active import show_active_boards
from utils.utils import Utility as Util
from utils.page.tab2_window import Tab2_window
from utils.page.tab3_window import Tab3_window
from utils.page.tab4_window import Tab4_window

import os
import sys
import json
import time
import shutil
import ctypes
from ctypes import *
import numpy as np


class MainWindow (QMainWindow):
    
    def __init__ (self):
        super().__init__()
        self.active_board   = QPixmap('data/src/img/active_panel.png')
        self.inactive_board = QPixmap('data/src/img/inactive_panel.png')

        # set app title, icon, and position 
        self.setWindowTitle("ASPET Software")
        self.setWindowIcon(QIcon("data/src/img/bime_logo.ico"))
        self.setGeometry(60, 110, 1000, 600)
        
        self.setStyleSheet
        ("""
            QMenuBar {
                background-color: rgb(240,240,240);
                color: rgb(49, 49, 49);
                border-bottom: 1px solid rgb(224,224,224);
            }

            QMenuBar::item {
                background-color: rgb(240,240,240);
                color: rgb(49, 49, 49);
                margin: 1px 10px 1px 2px;
                padding: 5px 5px 5px 5px;
            }

            QMenuBar::item::selected {
                background-color: rgb(200,200,200);
            }

            QMenu {
                left: 10px;
                background-color: rgb(224,224,224);
                color: rgb(49, 49, 49);
                border: 1px solid rgb(200,200,200)
            }

            QMenu::item::selected {
                background-color: rgb(200,200,200);
            }
        """)

        self.menu = self.menuBar()
        self.setTab1()
        self.setMenubar()
        
        self.processing_label = None
        self.processing_label = self.setLabel(self.processing_label, "", 460, 30)
        
        # - data information -
        self.active_boards = []
        self.datas = None
        self.config = dict()

        self.tab2_button = self.setButton('View Entries', (850, 100, 100, 50), self.setTab2)
        self.tab3_button = self.setButton('View Info',    (850, 200, 100, 50), self.setTab3)
        self.tab4_button = self.setButton('View Image',   (850, 300, 100, 50), self.setTab4)

        # - Initialize original state of main window - 
        self.init_window()

        # self.actionExit = QAction("Quit", self)
        # self.actionExit.triggered.connect(self.closeEvent)

        ### ====== ====== New file section of selecting input type with binary or simulation ====== ====== ###
        """
        [Mod] Insert other sources of simulation data
        1. New widget of selecting source input file (binary / simulation)
        2. Seperate the preprocessing procedure according to the input type
        3. Design new structure of config JSON for adapting different options
        """
        
        # ====== Layout: layout of file section ======
        self.file_panel = QWidget(self)  # Build widget for placing QLayout
        self.file_panel.setGeometry(320, 480, 380, 70)
        # - build QGridLayout -
        self.fileGrid = QGridLayout(self.file_panel)
        self.fileGrid.setContentsMargins(0, 0, 0, 0)
        # - Set layout of file section -
        self.file_panel.setLayout(self.fileGrid)

        # ====== widget: file type ======
        self.type_label = QLabel('Type:', self)
        self.fileGrid.addWidget(self.type_label, 0, 0)
        
        self.comboBox_type = QComboBox(self.file_panel)
        self.comboBox_type.setObjectName("comboBox_type")
        self.comboBox_type.addItems(['Binary','Simulation'])
        self.comboBox_type.setCurrentIndex(0)
        self.comboBox_type.currentIndexChanged.connect(self.type_onchanged)
        self.comboBox_type.setMinimumSize(100, 40)
        self.fileGrid.addWidget(self.comboBox_type, 1, 0)

        # ====== widget: file name ======
        self.fname_label = QLabel('File name:', self)
        self.fileGrid.addWidget(self.fname_label, 0, 1)

        self.comboBox_file = QComboBox(self.file_panel)
        self.comboBox_file.setObjectName("comboBox_file")
        self.comboBox_file.setMinimumSize(180, 40)
        self.fileGrid.addWidget(self.comboBox_file, 1, 1)
        
        # - File type onchange -
        self.type_onchanged()

        # ====== widget: action ======
        self.go_btn = QPushButton("Go", self)
        self.go_btn.clicked.connect(self.init_data)
        self.go_btn.setMinimumHeight(40)
        self.fileGrid.addWidget(self.go_btn, 1, 2)

        '''
        def closeEvent (self, event):
        reply = QMessageBox.question(self, 'Quit ?',
                                     'Are you sure you want to quit?',
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            if not type(event) == bool:
                app = QApplication.instance()
                app.closeAllWindows()
                event.accept()
            else:
                sys.exit()
        else:
            if not type(event) == bool:
                event.ignore()
        '''
    
    ### ====== ====== [Function] window widgets setup ====== ====== ###
    
    def setButton (self, text, location = None, function = None):
        button = QPushButton(text, self)  # creating a push button
        if location != None:
            button.setGeometry(location[0], location[1], location[2], location[3])  # setting geometry of button
        if function != None:
            button.clicked.connect(function)  # bind clicked function
        if self.datas == None:
            button.setEnabled(False)
        return button
    
    def setTab2 (self):
        self.tab2_window = Tab2_window()
        self.tab2_window.show()

    def setTab3 (self):
        self.tab3_window = Tab3_window(self.filepath)
        self.tab3_window.show()

    def setTab4 (self):
        self.tab4_window = Tab4_window()
        self.tab4_window.show()

    # - Show active boards and boards' ID -
    def settab1Label (self, label_name, text, x, y):
        label_name['label'] = QLabel(text, self)
        if int(text.split('Board')[-1]) >= 10:
            label_name['label'].move(x+5, y)
        else:
            label_name['label'].move(x+8, y)
        label_name['img'] = QLabel(self)
        label_name['img'].setPixmap(self.inactive_board)
        label_name['img'].move(x, y+30)
        label_name['img'].resize(65, 58)
        
    def setTab1 (self):
        self.tab1 = QLabel('Board Topology', self)
        self.tab1.resize(200, 30)
        self.tab1.move(29, 60)
        self.tab1.setFont(QFont('Time', 12, weight=QFont.Bold))
        self.board_labels = {}
        for i in range(4):
            for j in range(8):
                self.board_labels[i*8 + j] = {}
                board_name = "Board" + str(i*8 + j)
                x = 30 + j * 100
                y = 90 + i * 95
                self.settab1Label(self.board_labels[i*8 + j], board_name, x, y)
    
    # - Set 'File' menu bar -
    def setMenubar (self):
        file_menu = self.menu.addMenu("File")
        import_data = QAction(file_menu)
        import_data.setText("import data")
        import_data.triggered.connect(self.select_data)
        file_menu.addAction(import_data)

    def setLabel (self, label, text, x, y):
        label = QLabel(text, self)
        label.resize(200, 50)
        label.move(x, y)
        return label

    # - [onchange] switch file list when changing type -
    def type_onchanged (self):
        self.type = self.comboBox_type.currentText()
        # self.filename = self.comboBox_file.currentText()
        self.control_function_btn(False)
        
        if (self.type == "Binary"):
            # - Continue normal data extracting process
            # - (output) frame data, total info -> sort()
            self.list_binary = next(os.walk('data/bin/'))[2]
            self.comboBox_file.clear()
            self.comboBox_file.addItems(self.list_binary)
        if (self.type == "Simulation"):
            # Skip preprocessing step
            self.list_simulate = next(os.walk('data/simulation/'))[2]
            self.comboBox_file.clear()
            self.comboBox_file.addItems(self.list_simulate)
    
    ### ====== ====== [Function] data processing setup ====== ====== ###
    
    # - [control] init_data: after pressing the 'Go' button -
    def init_data (self):
        self.task = 'input'
        self.input_file = self.comboBox_file.currentText()
        self.type = self.comboBox_type.currentText()
        
        if (self.type == 'Binary'):
            Util.data_preprocess()
            self.show_active_boards()
            self.control_function_btn(True)
        
        if (self.type =='Simulation'):
            self.control_function_btn(True)
        
        Util.config_onchange()
    
    # - [!] update content of config file - ***
    def config_onchange_00 (self):
        # - Set configuration of input file in config.json -
        self.config['task'] = self.task
        self.config['name'] = self.input_file[:-4]
        self.config['type'] = self.type
        
        if self.task != 'input':
            self.config['sp'] = self.spacing
            self.config['vx'] = self.vx
            self.config['xp'] = self.xplane
        
        with open("data/src/tmp/config.json","w") as fConfig:
            json.dump(self.config, fConfig)
        fConfig.close()
    
    # ====== ====== ====== ====== ====== ====== ====== ====== ====== ====== ======
    # ====== ====== ====== ====== ====== ====== ====== ====== ====== ====== ======
    
    # - [!] old data preprocessing - ***
    def data_preprocess_00 (self):
        bin_file = self.comboBox_file.currentText()
        # (5) parse active boards and refresh icons  ====== ====== ====== ====== ***
        active = np.array(np.zeros(32), dtype=bool)

        with open('data/src/tmp/active.txt','r') as file:
            lines = file.readlines()
            for i, line in enumerate(lines):
                if i % 4 == 0:
                    a = int(line.split('\t')[0])
                    if a > 0:
                        active[i // 4] = True
        file.close()
        # - show active boards -
        for i, active in enumerate(self.active_boards):
            if active == True:
                self.board_labels[i]['img'].setPixmap(self.active_board)
        
    # ====== ====== ====== ====== ====== ====== ====== ====== ====== ====== ======
    # ====== ====== ====== ====== ====== ====== ====== ====== ====== ====== ======

    # - initial setting of main window -
    def init_window (self): 
        self.processing_label.setText('Extracting data ...')
        # - clean previous tmp folder for saving frame data -
        try:
            shutil.rmtree("data/src/tmp/")
        except:
            pass
        # - reset all board icons to deactive state -
        for i in range(32):
            self.board_labels[i]['img'].setPixmap(self.inactive_board)
    
    # - recording active boards in array -
    def show_active_boards (self):
        active = np.array(np.zeros(32), dtype=bool)

        with open('data/src/tmp/active.txt','r') as file:
            lines = file.readlines()
            for i, line in enumerate(lines):
                if i % 4 == 0:
                    a = int(line.split('\t')[0])
                    if a > 0:
                        active[i // 4] = True
        file.close()
        
        for i, active in enumerate(self.active_boards):
            if active == True:
                self.board_labels[i]['img'].setPixmap(self.active_board)

    # - [function] control state of function buttons -
    def control_function_btn (self, ifEnable):
        self.tab2_button.setEnabled(ifEnable)
        self.tab3_button.setEnabled(ifEnable)
        self.tab4_button.setEnabled(ifEnable)
        self.tab2_button.update()
        self.tab3_button.update()
        self.tab4_button.update()
        
    ### ====== [!] Browse data and show active boards ====== ###
    """
    ! Considering remove this function.
    ! New function data_preprocess() can deal with binary and simulation data.
    """
    def select_data (self):
        self.processing_label.setText('Extracting data ...')
        # - clean previous tmp folder for saving frame data -
        try:
            shutil.rmtree("data/src/tmp/")
        except:
            pass
        
        # - reset all board icons to deactive state -
        for i in range(32):
            self.board_labels[i]['img'].setPixmap(self.inactive_board)
        
        # - open file dialog for selecting input binary file -
        self.filepath, _ = QFileDialog.getOpenFileName(self, 'Select file', "data/bin/", filter="*.bin")

        if self.filepath:
            # - activate working boards icon -
            self.active_boards = show_active_boards(self.filepath)
            self.processing_label.setText("Finish extracting!")
            
            for i, active in enumerate(self.active_boards):
                if active == True:
                    self.board_labels[i]['img'].setPixmap(self.active_board)
            
            # - activate function buttons -
            self.tab2_button.setEnabled(True)
            self.tab3_button.setEnabled(True)
            self.tab4_button.setEnabled(True)
            self.tab2_button.update()
            self.tab3_button.update()
            self.tab4_button.update()
            
            # - save bin name to config file -
            self.config['bin'] = os.path.basename(self.filepath)[:-4]
            with open("data/src/tmp/config.json","w") as fConfig:
                json.dump(self.config, fConfig)
            fConfig.close()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
