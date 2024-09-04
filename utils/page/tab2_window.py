from PyQt5.QtCore import QSize, Qt
from PyQt5.QtWidgets import *
from PyQt5 import QtGui, QtWidgets
from PyQt5.QtGui import *

import matplotlib
matplotlib.use("Qt5Agg")
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as Canvas
from matplotlib.figure import Figure
from matplotlib.ticker import FuncFormatter

import numpy as np
from ..function.entries_distribution import entries_distribution

 
class MplCanvas (Canvas):
    def __init__ (self):
        self.fig = Figure(figsize=(8, 4), dpi=100)
        self.ax = self.fig.add_subplot(111)
        super(Canvas, self).__init__(self.fig)
        Canvas.setSizePolicy(self, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        Canvas.updateGeometry(self)


class MplWidget (QtWidgets.QWidget):
    def __init__ (self, parent = None):
        QtWidgets.QWidget.__init__(self, parent)   # Inherit from QWidget
        self.canvas = MplCanvas()                  # Create canvas object
        self.vbl = QtWidgets.QVBoxLayout()         # Set box for plotting
        self.vbl.addWidget(self.canvas)
        self.setLayout(self.vbl)


class Tab2_window (QMainWindow):
    def __init__ (self):
        super(Tab2_window, self).__init__()
        self.setWindowTitle("Entries view")
        self.setWindowIcon(QIcon("data/src/img/bime_logo.ico"))
        self.setGeometry(200, 200, 800, 600)

        self.board_combobox = {'active': True}
        self.setComboBox(self.board_combobox, 'Board ID', 200, 20, 32)
        self.board_combobox['combobox'].currentTextChanged.connect(self.board_combobox_changed)
        self.gmsl_combobox = {'active': False}
        self.setComboBox(self.gmsl_combobox, 'GMSL ID', 350, 20, 4)
        self.gmsl_combobox['combobox'].currentTextChanged.connect(self.gmsl_combobox_changed)
        self.stic_combobox = {'active': False}
        self.setComboBox(self.stic_combobox, 'STiC ID', 500, 20, 4)
        self.stic_combobox['combobox'].currentTextChanged.connect(self.stic_combobox_changed)

        self.entries = entries_distribution()
        
        self.figure = MplWidget(self)
        self.figure.resize(800, 400)
        self.figure.move(0, 150)
        self.setFigure()

    def setComboBox (self, combobox, text, x, y, num):
        combobox['label'] = QLabel(text, self)
        combobox['label'].move(x, y)
        combobox['combobox'] = QComboBox(self)
        combobox['combobox'].setEnabled(combobox['active'])
        items = ['All']
        for i in range(num):
            items.append(str(i))
        combobox['combobox'].addItems(items)
        combobox['combobox'].view().setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        combobox['combobox'].setStyleSheet('combobox-popup: 0;')
        combobox['combobox'].move(x, y + 30)

    def board_combobox_changed (self):
        if self.board_combobox['combobox'].currentText() == 'All':
            self.gmsl_combobox['combobox'].setEnabled(False)
            self.stic_combobox['combobox'].setEnabled(False)
        elif self.board_combobox['combobox'].currentText() != 'All':
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
        self.setFigure()

    def setFigure (self):
        self.figure.canvas.ax.cla()
        self.figure.canvas.draw_idle()

        def autolabel(rects, y):
            for idx,rect in enumerate(rects):
                height = rect.get_height()
                if y[idx] > 0:
                    self.figure.canvas.ax.text(rect.get_x() + rect.get_width()/2., height,
                                               y[idx], ha='center', va='bottom', rotation=0, c = 'blue', fontsize = 8)

        if self.board_combobox['combobox'].currentText() == 'All':
            x = [i for i in range(int(len(self.entries) / 4))]
            y = [int(self.entries[i].split('\t')[0]) for i in range(0, len(self.entries), 4)]
            label = [str(i) for i in range(int(len(self.entries) / 4))]
            bars = self.figure.canvas.ax.bar(x, y, width = 0.5, align = "center")
            autolabel(bars, y)
            self.figure.canvas.ax.set_title('Boards')
            self.figure.canvas.ax.tick_params(top = False, right = False)
            self.figure.canvas.ax.set_xticks(x)
            self.figure.canvas.ax.set_xticklabels(label, fontsize = 6)
        
        elif self.gmsl_combobox['combobox'].currentText() == 'All':
            board = int(self.board_combobox['combobox'].currentText())
            x = [i for i in range(4)]
            y = np.array(self.entries[board*4 + 1][:-1].split('\t'), dtype=np.uint32)
            label = [str(i) for i in x]
            bars = self.figure.canvas.ax.bar(x, y, width = 0.5, align = "center")
            autolabel(bars, y)
            self.figure.canvas.ax.set_title('Board ' + str(board) + ' GMSLs')
            self.figure.canvas.ax.tick_params(top = False, right = False)
            self.figure.canvas.ax.set_xticks(x)
            self.figure.canvas.ax.set_xticklabels(label, fontsize = 6)
        
        elif self.stic_combobox['combobox'].currentText() == 'All':
            board = int(self.board_combobox['combobox'].currentText())
            gmsl = int(self.gmsl_combobox['combobox'].currentText())
            x = [i for i in range(4)]
            y = np.array((self.entries[board*4 + 2][:-1].split('\t'))[gmsl*4: gmsl*4 + 4], dtype=np.uint32)
            label = [str(i) for i in x]
            bars = self.figure.canvas.ax.bar(x, y, width = 0.5, align = "center")
            autolabel(bars, y)
            self.figure.canvas.ax.set_title('Board ' + str(board) + ' GMSL ' + str(gmsl) + ' STiCs')
            self.figure.canvas.ax.tick_params(top = False, right = False)
            self.figure.canvas.ax.set_xticks(x)
            self.figure.canvas.ax.set_xticklabels(label, fontsize = 6)
        
        else:
            board = int(self.board_combobox['combobox'].currentText())
            gmsl = int(self.gmsl_combobox['combobox'].currentText())
            stic = int(self.stic_combobox['combobox'].currentText())
            x = [i for i in range(64)]
            y = np.array((self.entries[board*4 + 3][:-1].split('\t'))[(gmsl*4 + stic)*64: (gmsl*4 + stic)*64 + 64], dtype=np.uint32)
            label = [str(i) for i in x]
            bars = self.figure.canvas.ax.bar(x, y, width = 0.5, align = "center")
            self.figure.canvas.ax.set_title('Board ' + str(board) + ' GMSL ' + str(gmsl) + ' STiCs ' + str(stic) + ' Channels')
            self.figure.canvas.ax.tick_params(top = False, right = False)
            self.figure.canvas.ax.set_xticks(x)
            self.figure.canvas.ax.set_xticklabels(label, fontsize = 5)
