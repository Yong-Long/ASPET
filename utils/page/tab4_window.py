#!/usr/bin/python

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import QSize, Qt
from PyQt5 import QtGui, QtWidgets

import matplotlib
matplotlib.use("Qt5Agg")
from matplotlib.figure import Figure
from matplotlib.ticker import FuncFormatter
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as Canvas

import time
import json
import numpy as np
from ctypes import *

from ..utils import FBP_reconstruction, MLEM_reconstruction, MLEM_visualization, \
                    SRM_generate, ThreeAngle_reconstruction, ThreeAngle_visualization
from .config.MLEM_recon_cfg   import Ui_MLEMReconCfg
from .config.MLEM_vis_cfg     import Ui_MLEMVisCfg
from .config.SRM_gen_cfg      import Ui_SRMGenCfg
from .config.ThrAng_recon_cfg import Ui_ThrAngReconCfg
from .config.ThrAng_vis_cfg   import Ui_ThrAngVisCfg


class MLEMVisDialog (QtWidgets.QDialog, Ui_MLEMVisCfg):
    def __init__ (self, data, parent=None):
        super(MLEMVisDialog, self).__init__(parent)
        print(f"class MLEMVisDialog - data: {data}")
        self.nimgx = data[0]
        self.nimgy = data[1]
        self.nimgz = data[2]

class ThrAngVisDialog (QtWidgets.QDialog, Ui_ThrAngVisCfg):
    def __init__ (self, data, parent=None):
        super(ThrAngVisDialog, self).__init__(parent)
        print(f"class ThrAngVisDialog - data: {data}")
        self.nimgx = data[0]
        self.nimgy = data[1]
        self.nimgz = data[2]


class Tab4_window (QMainWindow):
    
    def __init__ (self):
        super(Tab4_window, self).__init__()
        self.setWindowTitle("Reconstruction")
        self.setWindowIcon(QIcon("data/src/img/bime_logo.ico"))
        self.setGeometry(170, 175, 1060, 650)

        # - Load config info from JSON file -
        self.config = self.parse_config()
        self.input_name = self.config['input']
        
        # ====== [Config] 3D imaging space ======
        # self.nimgx = int(60 / 0.8)  # [change] dynamically defined by SRM config (received from MLEM vis cfg)
        self.nimgz = 32 * 4
        self.nimgy = 16 * 4
        
        self.board_combobox = {'active': True}
        self.setComboBox(self.board_combobox, 'Board ID', 54, 20, 32)
        # self.board_combobox['combobox'].currentTextChanged.connect(self.board_combobox_changed)
        self.gmsl_combobox = {'active': True}
        self.setComboBox(self.gmsl_combobox, 'GMSL ID', 124, 20, 4)
        # self.gmsl_combobox['combobox'].currentTextChanged.connect(self.gmsl_combobox_changed)
        self.stic_combobox = {'active': True}
        self.setComboBox(self.stic_combobox, 'STiC ID', 194, 20, 4)
        # self.stic_combobox['combobox'].currentTextChanged.connect(self.stic_combobox_changed)
        
        # - Basic geometry of each STiC -
        self.default_channel_set = [[5 , 3 , 2 , 0 , 63, 61, 60, 58],
                                    [6 , 4 , 8 , 1 , 62, 55, 59, 57],
                                    [9 , 7 , 10, 12, 51, 53, 56, 54],
                                    [11, 13, 16, 14, 49, 47, 50, 52],
                                    [17, 15, 18, 20, 43, 45, 48, 46],
                                    [19, 21, 24, 22, 41, 39, 42, 44],
                                    [25, 23, 31, 29, 34, 32, 40, 38],
                                    [26, 27, 28, 30, 33, 35, 36, 37],]
        channel_sets = []        
        self.channels = [None] * 64
        self.channels_panel(self.channels)
        
        for i in range(4):
            for j in range(4):
                if (((i // 2) + (j // 2)) % 2) == 0:
                    # At board 0 (i=0,1), rotate 180 degrees if STiC lies on bottom row (j=0,1);
                    # At board 1 (i=2,3), rotate 180 degrees if STiC lies on bottom row (j=2,3).
                    channel_sets.append(np.rot90(self.default_channel_set, 2))
                else:
                    # At board 0 (i=0,1), remain orientation of STiC lying on top row (j=2,3);
                    # At board 1 (i=2,3), remain orientation of STiC lying on top row (j=0,1).
                    channel_sets.append(self.default_channel_set)
        
        # - Duplicate with 32 board pairs -
        self.channel_sets = [channel_sets] * 32
        
        # - Layout for channel setup - 
        self.button_layout = QHBoxLayout()
        
        # ====== default ======
        self.default_button = QPushButton("Default set", self)
        self.default_button.resize(100, 50)
        self.default_button.clicked.connect(self.set_default_channel)
        self.button_layout.addWidget(self.default_button)
        
        # ====== channel set ======
        self.set_button = QPushButton("Set", self)
        self.set_button.resize(100, 50)
        self.set_button.clicked.connect(self.set_channelset)
        self.button_layout.addWidget(self.set_button)
        
        # - Set position of channel layout -
        self.button_panel = QWidget(self)
        self.button_panel.setLayout(self.button_layout)
        self.button_panel.setGeometry(37, 330, 225, 50)
        
        # - Set grid layout of buttons -
        btnGrid = QGridLayout()
        btnGrid.setSpacing(5)
        
        # ====== ====== ====== FBP recon ====== ====== ======
        self.fbp_recon_btn = QPushButton("FBP recon", self)
        self.fbp_recon_btn.clicked.connect(self.FBP_recon)
        self.fbp_recon_btn.setMinimumHeight(50)
        btnGrid.addWidget(self.fbp_recon_btn, 1, 0)

        # ====== ====== ====== SRM generation ====== ====== ======
        self.srm_btn = QPushButton("SRM gen", self)
        self.srm_btn.clicked.connect(self.SRM_gen)
        self.srm_btn.setMinimumHeight(50)
        btnGrid.addWidget(self.srm_btn, 1, 1)
        
        # ====== ====== ====== MLEM recon ====== ====== ======
        self.mlem_recon_btn = QPushButton("ML-EM recon", self)
        self.mlem_recon_btn.clicked.connect(self.MLEM_recon)
        self.mlem_recon_btn.setMinimumHeight(50)
        btnGrid.addWidget(self.mlem_recon_btn, 2, 0)
        
        # ====== ====== ====== 3-angle recon ====== ====== ======
        self.thrang_recon_btn = QPushButton("3-angle recon", self)
        self.thrang_recon_btn.clicked.connect(self.ThreeAngle_recon)
        self.thrang_recon_btn.setMinimumHeight(50)
        btnGrid.addWidget(self.thrang_recon_btn, 2, 1)
        
        # ====== ====== ====== MLEM vis ====== ====== ======
        self.mlem_vis_btn = QPushButton("ML-EM vis", self)
        self.mlem_vis_btn.clicked.connect(self.MLEM_vis)
        self.mlem_vis_btn.setMinimumHeight(50)
        btnGrid.addWidget(self.mlem_vis_btn, 3, 0)
        
        # ====== ====== ====== 3-angle vis ====== ====== ======
        self.thrang_vis_btn = QPushButton("3-angle vis", self)
        self.thrang_vis_btn.clicked.connect(self.ThreeAngle_vis)
        self.thrang_vis_btn.setMinimumHeight(50)
        btnGrid.addWidget(self.thrang_vis_btn, 3, 1)
        
        # ====== ====== ====== Vis widget ====== ====== ======
        self.function_panel = QWidget(self)
        self.function_panel.setLayout(btnGrid)
        self.function_panel.setGeometry(37, 385, 240, 240)
        
        ### ====== ====== ====== Image panel ====== ====== ======
        style_grid = '''
                    background:#fff;
                    border:1px solid #000;
                    '''
        
        # ====== image label ======
        self.figttl_label = QLabel('Reconstructed Image', self)
        self.figttl_label.setGeometry(560, 30, 150, 20)
        self.figttl_label.setHidden(True)
        
        self.image_label = QLabel("", self)
        self.image_label.setAlignment(Qt.AlignCenter)
        self.image_label.setGeometry(300, 50, 720, 400)
        
        self.log_label = QLabel('', self)
        self.log_label = self.setLabel(self.log_label, "", 360, 450)
        self.log_label.resize(500, 30)
        self.log_label.setAlignment(Qt.AlignCenter)
        # self.log_label.setStyleSheet("border: 1px solid black;")
        
        # - Set grid layout of image widgets -
        visGrid = QGridLayout()
        visGrid.setSpacing(5)
        
        # ====== widget: axis ======
        self.axis_label = QLabel('Axis:', self)
        visGrid.addWidget(self.axis_label, 0, 0)
        
        self.axis_combobox = QComboBox(self)
        self.axis_combobox.addItems(['X','Y','Z'])
        self.axis_combobox.setCurrentIndex(0)
        self.axis_combobox.currentIndexChanged.connect(self.slice_onchanged)
        self.axis_combobox.setMinimumSize(100, 30)
        visGrid.addWidget(self.axis_combobox, 0, 1)
        
        # ====== widget: acq ======
        self.acq_label = QLabel('Acq:', self)
        visGrid.addWidget(self.acq_label, 0, 3)

        self.acq_combobox = QComboBox(self)
        self.acq_combobox.addItems(['1','2','3'])
        self.acq_combobox.setCurrentIndex(1)
        self.acq_combobox.currentIndexChanged.connect(self.slice_onchanged)
        visGrid.addWidget(self.acq_combobox, 0, 4)
        
        # ====== widget: slice ======
        self.slice0_label = QLabel('Slice:', self)
        visGrid.addWidget(self.slice0_label, 1, 0)
        
        self.slice_slider = QSlider(self)
        self.slice_slider.setOrientation(1)
        self.slice_slider.valueChanged.connect(self.slice1_onchanged)
        visGrid.addWidget(self.slice_slider, 1, 1, 1, 4)
        
        self.slice1_label = QLabel(str(self.slice_slider.value()), self)
        self.slice1_label.setAlignment(Qt.AlignCenter)
        self.slice1_label.setMinimumSize(100, 12)
        visGrid.addWidget(self.slice1_label, 2, 1, 2, 4)
        
        self.vis_panel = QWidget(self)
        self.vis_panel.setLayout(visGrid)
        self.vis_panel.setGeometry(420, 500, 400, 100)
        # self.vis_panel.setStyleSheet(style_grid)
        self.visPanel_setHidden(True, True)  # deactivate widgets of vis panel
        
        self.slice_onchanged()
    
    def slice_onchanged (self):
        self.axis  = self.axis_combobox.currentText()
        self.nimgx = 320

        if self.axis == 'X':
            self.xlabel='Z'
            self.ylabel='Y'
            self.slice_slider.setRange(1, self.nimgx)
            self.slice_slider.setValue(self.nimgx // 2)
        elif self.axis == 'Y':
            self.xlabel='Z'
            self.ylabel='X'
            self.slice_slider.setRange(1, self.nimgy)
            self.slice_slider.setValue(self.nimgy // 2)
        elif self.axis == 'Z':
            self.xlabel='X'
            self.ylabel='Y'
            self.slice_slider.setRange(1, self.nimgz)
            self.slice_slider.setValue(self.nimgz // 2)
        
    def slice1_onchanged (self):
        self.slice = self.slice_slider.value()
        self.slice1_label.setText(str(self.slice))
    
    '''
    def setTab4config (self):
        self.tab4_setconfig = Tab4_setConfig()
        self.tab4_setconfig.show()
    '''

    def channels_panel (self, channels, loc = None):
        if loc == None:
            self.channels_label = QLabel('Channel Set', self)
            self.channels_label.move(110, 80)
            for i in range(64):
                channels[i] = QLineEdit(self)
                channels[i].setMaxLength(2)
                channels[i].setFixedWidth(25)
                channels[i].setText(str(self.default_channel_set[i // 8][i % 8]))
                channels[i].move(50 + (i % 8) * 25, 110 + (i // 8) * 25)
        else:
            for i in range(64):
                channels[i].setText(str(loc[i // 8][i % 8]))
                channels[i].update()
            
    def setComboBox (self, combobox, text, x, y, num):
        combobox['label'] = QLabel(text, self)
        combobox['label'].move(x, y)
        combobox['combobox'] = QComboBox(self)
        combobox['combobox'].setEnabled(combobox['active'])
        items = []
        for i in range(num):
            items.append(str(i))
        combobox['combobox'].addItems(items)
        combobox['combobox'].view().setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        combobox['combobox'].setStyleSheet('combobox-popup: 0;')
        combobox['combobox'].move(x, y + 30)
        combobox['combobox'].resize(50,30)

    def setLabel (self, label, text, x, y):
        label = QLabel(text, self)
        label.resize(200, 50)
        label.move(x, y)
        return label

    def set_channelset (self):
        board = int(self.board_combobox['combobox'].currentText())
        gmsl  = int(self.gmsl_combobox['combobox'].currentText())
        stic  = int(self.stic_combobox['combobox'].currentText())
        
        channel_sets = np.zeros((8, 8))
        for i in range(64):
            channel_sets[i // 8][i % 8] = int(self.channels[i].text())
        if ((gmsl // 2) + (stic // 2)) // 2 == 0:
            self.channel_sets[board][gmsl * 4 + stic] = np.rot90(channel_sets, 2)
        else:
            self.channel_sets[board][gmsl * 4 + stic] = channel_sets

    def set_default_channel (self):
        board = int(self.board_combobox['combobox'].currentText())
        gmsl  = int(self.gmsl_combobox['combobox'].currentText())
        stic  = int(self.stic_combobox['combobox'].currentText())
        
        if ((gmsl // 2) + (stic // 2)) % 2 == 0:
            self.channel_sets[board][gmsl * 4 + stic] = np.rot90(self.default_channel_set, 2)
        else:
            self.channel_sets[board][gmsl * 4 + stic] = self.default_channel_set
        self.channels_panel(self.channels, self.default_channel_set)
    
    # - Parse parameter setting from config file -
    def parse_config (self):
        config_path = "data/src/tmp/config.json"
        with open(config_path, "r") as fConfig:
            config_dict = json.load(fConfig)
        return config_dict
    
    
    ### ====== ====== Filtered Back-Projection reconstruction ====== ======
    def FBP_recon (self):
        self.visPanel_setHidden(True, True)
        self.log_label.setText('Running FBP recon ...')
        self.figttl_label.setHidden(False)
        # board = int(self.board_combobox['combobox'].currentText())
        FBP_reconstruction(self)
        self.setFigure("FBP_recon")
        
    
    ### ====== ====== ML-EM reconstruction ====== ======
    def MLEM_recon (self):
        self.QDialogConf = QtWidgets.QDialog()
        self.ui_mlemreconcfg = Ui_MLEMReconCfg()
        self.ui_mlemreconcfg.setupUi(self.QDialogConf)
        response = self.QDialogConf.exec_()
        
        if response:
            iterations = self.ui_mlemreconcfg.spinBox_iter.text()
            xplanes    = self.ui_mlemreconcfg.spinBox_xp.text()
            spacing    = self.ui_mlemreconcfg.lineEdit_sp.text()
            vx         = self.ui_mlemreconcfg.lineEdit_vx.text()
            
            MLEM_reconstruction(self, self.input_name, iterations, spacing, vx, xplanes)
    
    
    ### ====== ====== 3-angle reconstruction ====== ======
    def ThreeAngle_recon (self):
        self.QDialogConf = QtWidgets.QDialog()
        self.ui_thrangreconcfg = Ui_ThrAngReconCfg()
        self.ui_thrangreconcfg.setupUi(self.QDialogConf)
        response = self.QDialogConf.exec_()
        
        if response:
            input_3ang = self.ui_thrangreconcfg.comboBox_input.currentText()
            iterations = self.ui_thrangreconcfg.spinBox_iter.text()
            xplanes    = self.ui_thrangreconcfg.spinBox_xp.text()
            spacing    = self.ui_thrangreconcfg.lineEdit_sp.text()
            vx = self.ui_thrangreconcfg.lineEdit_vx.text()
            
            ThreeAngle_reconstruction(self, input_3ang, iterations, spacing, vx, xplanes)

    
    ### ====== ====== Visualization of MLEM recon ====== ======
    def MLEM_vis (self):
        self.nimgx_mlem = 320  # Can be adjusted in MLEM vis cfg
        self.nimgz_mlem = 32 * 4
        self.nimgy_mlem = 16 * 4
        self.data_mlem  = [self.nimgx_mlem, self.nimgy_mlem, self.nimgz_mlem]
        print(f"Tab4 MLEM_vis - self.data_mlem: {self.data_mlem}")
        
        self.QDialogConf   = QtWidgets.QDialog()
        self.ui_mlemviscfg = MLEMVisDialog(self.data_mlem)
        self.ui_mlemviscfg.setupUi(self.QDialogConf)
        response = self.QDialogConf.exec_()
        
        if response:
            self.visPanel_setHidden(True)  # deactivate vis panel
            self.figttl_label.setHidden(False)
            
            self.datn_mlem    = self.ui_mlemviscfg.comboBox_mlem.currentText()
            self.iters_mlem   = self.ui_mlemviscfg.spinBox_iter.text()
            self.axis_mlem    = self.ui_mlemviscfg.comboBox_axis.currentText()
            self.spacing_mlem = self.ui_mlemviscfg.spinBox_sp.text()
            self.slice_mlem   = self.ui_mlemviscfg.horizSlider_slice.value()
            # - nimgx, axis, slice (, norm) -
            self.nimgx_mlem   = int(float(self.spacing_mlem) / 0.8) #+ 0.5  # Determined by the SRM config
            self.axis_combobox.setCurrentText(self.axis_mlem)
            self.slice_slider.setValue(self.slice_mlem)
            self.slice_slider.setRange(1, self.nimgx_mlem)
            
            self.visPanel_setHidden(False)  # activate MLEM vis panel
            
            self.axis_combobox.currentIndexChanged.connect(self.mlem_vis_onchanged)
            self.slice_slider.valueChanged.disconnect()
            self.slice_slider.valueChanged.connect(self.mlem_vis_onchanged)
            # self.log_label.setStyleSheet("border: 1px solid black;")
            # print(f"self.datn_mlem: {self.datn_mlem}, iters_mlem:{self.iters_mlem}, slice_mlem:{self.slice_mlem}")
            
            self.mlem_vis_onchanged()
            self.setFigure("MLEM_recon")

    
    ### ====== ====== Visualization of 3-angle recon ====== ======
    def ThreeAngle_vis (self):        
        self.nimgx_3ang = 320
        self.nimgy_3ang = 16 * 4
        self.nimgz_3ang = 32 * 4
        self.data_3ang  = [self.nimgx_3ang, self.nimgy_3ang, self.nimgz_3ang]
        print(f"Tab4 ThreeAngle_vis - self.data_3ang: {self.data_3ang}")
        
        self.QDialogConf = QtWidgets.QDialog()
        self.ui_thrangviscfg = ThrAngVisDialog(self.data_3ang)
        self.ui_thrangviscfg.setupUi(self.QDialogConf)
        response = self.QDialogConf.exec_()
        
        if response:
            self.visPanel_setHidden(True, True)  # deactivate 3-angle vis panel
            self.figttl_label.setHidden(False)
            self.datn_3ang    = self.ui_thrangviscfg.comboBox_3angle.currentText()
            self.acq_3ang     = self.ui_thrangviscfg.comboBox_acq.currentText()
            self.iters_3ang   = self.ui_thrangviscfg.spinBox_iter.text()
            self.spacing_3ang = self.ui_thrangviscfg.spinBox_sp.text()
            self.axis_3ang    = self.ui_thrangviscfg.comboBox_axis.currentText()
            self.slice_3ang   = self.ui_thrangviscfg.horizSlider_slice.value()
            # - nimgx, axis, slice -
            self.nimgx_3ang   = int(float(self.spacing_3ang) / 0.8)  # Determined by the SRM config
            self.axis_combobox.setCurrentText(self.axis_3ang)
            self.slice_slider.setValue(self.slice_3ang)
            self.slice_slider.setRange(1, self.nimgx_3ang)
            
            self.visPanel_setHidden(False, False)  # activate 3-angle vis panel
            self.acq_combobox.setHidden(False)
            
            self.axis_combobox.currentIndexChanged.connect(self.thrang_vis_onchanged)
            self.acq_combobox.currentIndexChanged.connect(self.thrang_vis_onchanged)
            self.slice_slider.valueChanged.disconnect()
            self.slice_slider.valueChanged.connect(self.thrang_vis_onchanged)
            
            self.thrang_vis_onchanged()
            self.setFigure("ThreeAngle_recon")

    # - Update MLEM image results -
    def mlem_vis_onchanged (self):
        self.slice1_label.setText(str(self.slice_slider.value()))
        self.axis_mlem  = self.axis_combobox.currentText()
        self.slice_mlem = self.slice_slider.value()

        MLEM_visualization(self)
        self.setFigure("MLEM_recon")
    
    # - Update 3-angle image results -
    def thrang_vis_onchanged (self):
        self.slice1_label.setText(str(self.slice_slider.value()))
        self.axis_3ang  = self.axis_combobox.currentText()
        self.acq_3ang   = self.acq_combobox.currentText()
        self.slice_3ang = self.slice_slider.value()
        
        ThreeAngle_visualization(self)
        self.setFigure("ThreeAngle_recon")
        
    
    ### ====== ====== Generate SRM files ====== ======
    def SRM_gen (self):
        self.QDialogConf  = QtWidgets.QDialog()
        self.ui_srmgencfg = Ui_SRMGenCfg()
        self.ui_srmgencfg.setupUi(self.QDialogConf)
        response = self.QDialogConf.exec_()
        
        if response:
            self.spacing = self.ui_srmgencfg.lineEdit_sp.text()
            self.vx      = self.ui_srmgencfg.lineEdit_vx.text()
            self.xplanes = self.ui_srmgencfg.spinBox_xp.text()
            self.log_label.setText(f'Running SRM gen with spacing {self.spacing}mm, vx {self.vx}mm and {self.xplanes} planes ...')
            
            SRM_generate(self)
        
    
    ### ====== ====== Control visibility of vis widgets ====== ======
    def visPanel_setHidden (self, ifHidden=True, ifThrAng=True):
        self.log_label.setHidden(ifHidden)
        self.axis_label.setHidden(ifHidden)
        self.axis_combobox.setHidden(ifHidden)
        self.acq_label.setHidden(ifThrAng)
        self.acq_combobox.setHidden(ifThrAng)
        self.slice_slider.setHidden(ifHidden)
        self.slice0_label.setHidden(ifHidden)
        self.slice1_label.setHidden(ifHidden)
    
    
    ### ====== ====== Show output figure on panel ====== ======
    def setFigure (self, image):
        pixmap = QPixmap(f'data/src/tmp/{image}.png')
        # pixmap.scaledToWidth(self.image_label.width(), Qt.SmoothTransformation)
        # pixmap.scaledToHeight(self.image_label.height(), Qt.SmoothTransformation)
        self.image_label.setPixmap(pixmap)
        # self.image_label.setScaledContents(False)