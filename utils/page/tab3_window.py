from PyQt5.QtCore import QSize, Qt
from PyQt5.QtWidgets import *
from PyQt5 import QtGui, QtWidgets, QtCore
from PyQt5.QtGui import *

from ctypes import *
import matplotlib
matplotlib.use("Qt5Agg")
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as Canvas
from matplotlib.figure import Figure
from matplotlib.ticker import FuncFormatter

from ..function.range_slider import RangeSlider
from ..function.energy_info import read_energy_info, read_record, show_energy_info


class MplCanvas (Canvas):
    def __init__ (self):
        self.fig = Figure(figsize=(8, 4), dpi=100)
        self.ax = self.fig.add_subplot(111)
        super(Canvas, self).__init__(self.fig)
        Canvas.setSizePolicy(self, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        Canvas.updateGeometry(self)


class MplWidget (QtWidgets.QWidget):
    def __init__ (self, parent=None):
        QtWidgets.QWidget.__init__(self, parent)   # Inherit from QWidget
        self.canvas = MplCanvas()                  # Create canvas object
        self.vbl = QtWidgets.QVBoxLayout()         # Set box for plotting
        self.vbl.addWidget(self.canvas)
        self.setLayout(self.vbl)


class Tab3_window (QWidget):
    def __init__ (self, filepath = None):
        super(Tab3_window, self).__init__()
        self.setWindowTitle("Channel Info")
        self.setWindowIcon(QIcon("data/src/img/bime_logo.ico"))
        self.setGeometry(200, 200, 1200, 600)

        self.filepath = filepath
        self.entries  = read_record()

        self.board_combobox = {'active': True}
        self.setComboBox(self.board_combobox, 'Board ID', 225, 20, 32, 'None')
        self.board_combobox['combobox'].currentTextChanged.connect(self.board_combobox_changed)
        self.gmsl_combobox = {'active': False}
        self.setComboBox(self.gmsl_combobox, 'GMSL ID', 375, 20, 4)
        self.gmsl_combobox['combobox'].currentTextChanged.connect(self.gmsl_combobox_changed)
        self.stic_combobox = {'active': False}
        self.setComboBox(self.stic_combobox, 'STiC ID', 525, 20, 4)
        self.stic_combobox['combobox'].currentTextChanged.connect(self.stic_combobox_changed)

        self.channel_menu = QLabel("Channel Menu", self)
        self.channel_menu.move(675, 20)
        self.radiobuttons0 = QWidget(self)
        self.channel_menu_layout = QHBoxLayout()
        self.channel_h_layout1 = QVBoxLayout()
        self.channel_h_layout2 = QVBoxLayout()
        self.set0 = QRadioButton("Set 0")
        self.set0.setEnabled(False)
        self.set0.toggled.connect(self.setSet)
        self.set1 = QRadioButton("Set 1")
        self.set1.setEnabled(False)
        self.set1.toggled.connect(self.setSet)
        self.set2 = QRadioButton("Set 2")
        self.set2.setEnabled(False)
        self.set2.toggled.connect(self.setSet)
        self.set3 = QRadioButton("Set 3")
        self.set3.setEnabled(False)
        self.set3.toggled.connect(self.setSet)
        self.channel_h_layout1.addWidget(self.set0)
        self.channel_h_layout1.addWidget(self.set2)
        self.channel_h_layout2.addWidget(self.set1)
        self.channel_h_layout2.addWidget(self.set3)
        self.channel_menu_layout.addLayout(self.channel_h_layout1)
        self.channel_menu_layout.addLayout(self.channel_h_layout2)
        self.radiobuttons0.setLayout(self.channel_menu_layout)
        self.radiobuttons0.move(652, 35)
        self.set_num = None

        self.mode_menu = QLabel("Mode Menu", self)
        self.mode_menu.move(883, 20)
        self.radiobuttons1 = QWidget(self)
        self.mode_menu_layout = QHBoxLayout()
        self.mode_h_layout1 = QVBoxLayout()
        self.mode_h_layout2 = QVBoxLayout()
        self.settcc = QRadioButton("tcc")
        self.settcc.setEnabled(True)
        self.settcc.toggled.connect(self.setMode)
        self.setecc = QRadioButton("ecc")
        self.setecc.setEnabled(True)
        self.setecc.toggled.connect(self.setMode)
        self.setenergy = QRadioButton("energy")
        self.setenergy.setEnabled(True)
        self.setenergy.toggled.connect(self.setMode)
        self.setaaa = QRadioButton("???")
        self.setaaa.setEnabled(True)
        self.setaaa.toggled.connect(self.setMode)
        self.mode_h_layout1.addWidget(self.settcc)
        self.mode_h_layout1.addWidget(self.setenergy)
        self.mode_h_layout2.addWidget(self.setecc)
        self.mode_h_layout2.addWidget(self.setaaa)
        self.mode_menu_layout.addLayout(self.mode_h_layout1)
        self.mode_menu_layout.addLayout(self.mode_h_layout2)
        self.radiobuttons1.setLayout(self.mode_menu_layout)
        self.radiobuttons1.move(850, 35)
        self.mode = None
        
        # - slider -
        self.slider_label = QLabel("Threshold Range", self)
        self.slider_label.move(868, 110)
        self.slider_widget = QWidget(self)
        self.slider_total_layout = QVBoxLayout()
        self.slider_layout = QHBoxLayout()
        self.slider_layout2 = QHBoxLayout()
        self.slider = RangeSlider(QtCore.Qt.Horizontal)
        self.slider.setMinimumHeight(15)
        self.slider.setMinimum(0)
        self.slider.setMaximum(32767)
        self.slider.setLow(0)
        self.slider.setHigh(1000)
        self.slider.sliderMoved.connect(self.setThreshold)
        # self.slider.setTickPosition(QtWidgets.QSlider.TicksBelow)

        self.min = QLineEdit()
        self.min.setMaxLength(5)
        self.min.setFixedWidth(50)
        self.min.setPlaceholderText("Min")
        self.min.returnPressed.connect(self.setMin)
        self.max = QLineEdit()
        self.max.setMaxLength(5)
        self.max.setFixedWidth(50)
        self.max.setPlaceholderText("Max")
        self.max.returnPressed.connect(self.setMax)
        self.slider_layout2.addWidget(self.min)
        self.slider_layout2.addWidget(self.max)
        self.slider_layout.addWidget(self.slider)
        self.slider_total_layout.addLayout(self.slider_layout)
        self.slider_total_layout.addLayout(self.slider_layout2)
        self.slider_widget.setLayout(self.slider_total_layout)
        self.slider_widget.move(858, 125)

        # self.entries = entries
        # read_energy_info()
        # read_record()

        self.figure = MplWidget(self)
        self.figure.resize(600, 400)
        self.figure.move(200, 150)
        self.setFigure()
    
    def setThreshold (self, low_value, high_value):
        self.min.setText(str(low_value))
        self.max.setText(str(high_value))
        self.setFigure()
    
    def setMin (self):
        if self.min.text() != '' and self.max.text() != '':
            if int(self.max.text()) <= int(self.min.text()):
                pass
            else:
                self.slider.setLow(int(self.min.text()))
                # self.slider.update()
                self.setFigure()
    
    def setMax (self):
        if self.max.text() != '' and self.min.text() != '':
            if int(self.min.text()) >= int(self.max.text()):
                pass
            else:
                self.slider.setHigh(int(self.max.text()))
                # self.slider.update()
                self.setFigure()

    def setSet (self):
        radioBtn = self.sender()
        if radioBtn.isChecked():
            self.set_num = radioBtn.text().split(" ")[1]
            self.setFigure()
    
    def setMode (self):
        self.board_combobox['combobox'].setCurrentText('None')
        radioBtn = self.sender()
        if radioBtn.isChecked():
            self.mode = radioBtn.text()
            read_energy_info(self.filepath, self.mode)

    def setComboBox (self, combobox, text, x, y, num, first_item = 'All'):
        combobox['label'] = QLabel(text, self)
        combobox['label'].move(x, y)
        combobox['combobox'] = QComboBox(self)
        combobox['combobox'].setEnabled(combobox['active'])
        items = [first_item]
        for i in range(num):
            items.append(str(i))
        combobox['combobox'].addItems(items)
        combobox['combobox'].view().setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        combobox['combobox'].setStyleSheet('combobox-popup: 0;')
        combobox['combobox'].move(x, y + 20)

    def board_combobox_changed (self):
        if self.board_combobox['combobox'].currentText() == 'None':
            self.gmsl_combobox['combobox'].setEnabled(False)
            self.stic_combobox['combobox'].setEnabled(False)
        else:
            self.gmsl_combobox['combobox'].setEnabled(True)
        self.gmsl_combobox['combobox'].setCurrentText('All')
        self.stic_combobox['combobox'].setCurrentText('All')
        self.setFigure()

    def gmsl_combobox_changed (self):
        if self.gmsl_combobox['combobox'].currentText() == 'All':
            self.stic_combobox['combobox'].setEnabled(False)
        else:
            self.stic_combobox['combobox'].setEnabled(True)
        self.stic_combobox['combobox'].setCurrentText('All')
        self.setFigure()

    def stic_combobox_changed (self):
        if self.stic_combobox['combobox'].currentText() == 'All':
            self.set0.setEnabled(False)
            self.set1.setEnabled(False)
            self.set2.setEnabled(False)
            self.set3.setEnabled(False)
            self.set_num = None
        else:
            self.set0.setEnabled(True)
            self.set1.setEnabled(True)
            self.set2.setEnabled(True)
            self.set3.setEnabled(True)
        self.setFigure()

    def setFigure (self):
        self.figure.canvas.ax.cla()
        self.figure.canvas.draw_idle()
        
        if self.mode != None:
            if self.board_combobox['combobox'].currentText() == 'None':
                pass
            
            elif self.board_combobox['combobox'].currentText() != 'None' and self.gmsl_combobox['combobox'].currentText() == 'All':
                self.figure.canvas.fig.clf()
                self.figure.canvas.ax = self.figure.canvas.fig.add_subplot(111)
                board_num = self.board_combobox['combobox'].currentText()
                y = show_energy_info(record = self.entries, mode = self.mode, board_num = board_num)
                try:
                    min = int(self.min.text())
                    max = int(self.max.text())
                    self.figure.canvas.ax.hist(y, bins=range(min, max, 2), range=(min, max), density = False, histtype='step')
                except:
                    self.figure.canvas.ax.hist(y, bins=range(0, 1000, 2),range=(0, 1000), density = False, histtype='step')
                self.figure.canvas.ax.set_title('Board' + board_num)
                self.figure.canvas.ax.tick_params(top = False, right = False)
            
            elif self.gmsl_combobox['combobox'].currentText() != 'All' and self.stic_combobox['combobox'].currentText() == 'All':
                self.figure.canvas.fig.clf()
                self.figure.canvas.ax = self.figure.canvas.fig.add_subplot(111)
                board_num = self.board_combobox['combobox'].currentText()
                gmsl_num = self.gmsl_combobox['combobox'].currentText()
                y = show_energy_info(record = self.entries, mode = self.mode, board_num = board_num, gmsl_num = gmsl_num)
                try:
                    min = int(self.min.text())
                    max = int(self.max.text())
                    self.figure.canvas.ax.hist(y, bins=range(min, max, 2), range=(min, max), density = False, histtype='step')
                except:
                    self.figure.canvas.ax.hist(y, bins=range(0, 1000, 2),range=(0, 1000), density = False, histtype='step')
                self.figure.canvas.ax.set_title('Board' + board_num + ' GMSL' + gmsl_num)
                self.figure.canvas.ax.tick_params(top = False, right = False)
            
            elif self.stic_combobox['combobox'].currentText() != 'All' and self.set_num == None:
                self.figure.canvas.fig.clf()
                self.figure.canvas.ax = self.figure.canvas.fig.add_subplot(111)
                board_num = self.board_combobox['combobox'].currentText()
                gmsl_num = self.gmsl_combobox['combobox'].currentText()
                stic_num = self.stic_combobox['combobox'].currentText()
                y = show_energy_info(record = self.entries, mode = self.mode, board_num = board_num, gmsl_num = gmsl_num, stic_num = stic_num)
                try:
                    min = int(self.min.text())
                    max = int(self.max.text())
                    self.figure.canvas.ax.hist(y, bins=range(min, max, 2), range=(min, max), density = False, histtype='step')
                except:
                    self.figure.canvas.ax.hist(y, bins=range(0, 1000, 2),range=(0, 1000), density = False, histtype='step')
                self.figure.canvas.ax.set_title('Board' + board_num + ' GMSL' + gmsl_num + ' STiC' + stic_num)
                self.figure.canvas.ax.tick_params(top = False, right = False)
            
            elif self.set_num != None:
                self.figure.canvas.fig.clf()
                board_num = self.board_combobox['combobox'].currentText()
                gmsl_num = self.gmsl_combobox['combobox'].currentText()
                stic_num = self.stic_combobox['combobox'].currentText()
                y = show_energy_info(record = self.entries, mode = self.mode, board_num = board_num, gmsl_num = gmsl_num, stic_num = stic_num, channel_set = self.set_num)
                self.figure.canvas.ax.spines['right'].set_visible(False)
                self.figure.canvas.ax.spines['bottom'].set_visible(False)
                self.figure.canvas.ax.spines['left'].set_visible(False)
                self.figure.canvas.ax.spines['top'].set_visible(False)
                self.figure.canvas.ax.tick_params(axis='x', colors='white', grid_alpha=1)
                self.figure.canvas.ax.tick_params(axis='y', colors='white', grid_alpha=1)
                self.figure.canvas.ax.margins(x=2, y=2)
                fig_title = "Board" + board_num + " GMSL" + gmsl_num + " STiC" + stic_num
                self.figure.canvas.fig.suptitle(fig_title, fontsize=12)
                for i in range(4):
                    for j in range(4):
                        ax = self.figure.canvas.fig.add_subplot(4, 4, 4*i + j + 1)
                        ax.margins(0.01)
                        try:
                            min = int(self.min.text())
                            max = int(self.max.text())
                            ax.hist(y[4*i + j], bins=range(min, max, 2), range=(min, max), density = False, histtype='step')
                        except:
                            ax.hist(y[4*i + j], bins=range(0, 1000, 2),range=(0, 1000), density = False, histtype='step')
                        title = "Channel " + str(int(self.set_num) * 16 + 4*i + j)
                        ax.set_title(title, fontsize=6)
                        ax.tick_params(top = False, right = False)
                        ax.tick_params(axis='x', labelsize=6)
                        ax.tick_params(axis='y', labelsize=6)
                self.figure.canvas.fig.tight_layout()
