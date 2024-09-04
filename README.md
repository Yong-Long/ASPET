# ASPET Software

## Installation

Before starting on the project `ASPET software`,
there are some preparations should be finished ahead,
including validating GitHub connection and cloning ASPET GitHub repository.

> This project is developed on Ubuntu 24.04 (VMware Workstation 17 Player).
> You can execute the program on both Linux server or local virtual machine.

![ASPET VM](data/src/README/neofetch_Ubuntu22.04.png)

### a. Validate GitHub connection

Since this GitHub repo is built privately, you should connect repository with SSH key.
Please follow the instructions of [README_SSH_config.md](data/src/README/README_SSH_config.md),
then you can access the repo properly.

### b. Clone ASPET GitHub repository

The main versions of the software can be installed through the following command:

```bash
git clone https://github.com/Yong-Long/ASPET_software.git
```

If you want to run on Windows system, here's a testing version (not latest).

```bash
git clone -b win-version https://github.com/Yong-Long/ASPET_software.git
```

After downloading, you should see the home directory as below:
![ASPET home directory](data/src/README/ASPET_home_dir.png)

------------------------------------------------------------------------------

## Environment setup

### 1. Create and activate virtual environment

For the stability of program, it is recommended to develop a virtual environment.
Please choose a way you prefer to build venv by Anaconda or native Python.

#### a. Build venv by Anaconda (Recommend)

```bash
# [b] Create Anaconda venv
conda create --name aspet
# - Activate Anaconda venv -
conda activate aspet
```

#### b. Build venv by Python

```bash
# [a] Create Python virtualenv
mkdir ~/VENV && cd ~/VENV
python3 -m venv aspet
# - Activate Python virtualenv -
source ~/VENV/aspet/bin/activate

# - Or you can create a shortcut by adding alias within the .bashrc -
# (1) open and edit .bashrc
vim ~/.bashrc
# (2) add alias in .bashrc and save
alias venvASPET="source ~/VENV/aspet/bin/activate"
# (3) execute .bashrc
source ~/.bashrc
# (4) next time, you can activate venv by the alias term
venvASPET
```

### 2. Install required Python packages

There are some Python packages required for the program,
and you can directly install them by:

```bash
pip install -r requirements.txt
```

### 3. Deploy required SRM files for ML-EM

Since the SRM files are too large to upload to GitHub repo,
they have to be introduced first before running ML-EM program.
You can generate those by `SRM gen` function in Tab4;

Otherwise, you can also append it from outter file source,
such as unzipping SRM files into `data/SRM/` folder manually.

For example, download the `_srm_sp60_vx0.8_x75.zip` from shared Google drive,
and use following commands to upzip files to repective folders:

```bash
unzip ~/Downloads/_srm_sp60_vx0.8_x75.zip
unzip ~/Downloads/_srm_sp256_vx0.8_x320.zip
mv ~/Downloads/_srm_sp60_vx0.8_x75/ data/SRM/
mv ~/Downloads/_srm_sp256_vx0.8_x320/ data/SRM/
```

### 4. Complie C++ shared library

If you just initiate this project or make any change on C++ sharelib,
please rebuild `.so` file with:

```bash
source utils/function/build_sharelib.sh
```

------------------------------------------------------------------------------

## Getting start

### Main window

After activating venv `aspet`, then execute ASPET software by:

```bash
python3 main_window.py
```

![Main window](data/src/README/main_window.png)

It’ll open the PyQt5 main interface,
before reconstruction, please choose input file through the widget below.
Or you can click `File` -> `import data` to input the binary file (old way).

![Main data bin](data/src/README/Main_data_bin.png)

There are 4 sample bin files in the folder, please choose one to process.
> The 3rd one is empty. And you can choose other data in `new_bin/` folder.
> If you want to add more bin data in the software, please upload into `data/bin/`.

![Main choose bin](data/src/README/Main_choose_bin.png)

The bin file would be processed by `extract_data.so` and `sorting.so` in `utils/lib/`,
generating `active.txt`, `LOR_B0B1_<bin>.txt` and `frame_info.txt` in `data/src/tmp/`.
Then the main window shows icons to indicate which board is active.
Meanwhile, the 3 buttons on right side are also activated.

![Main active board](data/src/README/main_window_active.png)

### Tab2 window: view entry

Press `View Entries` button from main window, you'll get into Tab2 window.
This function is to check event activity in global view.
Through the combobox on top row, you can view over different dimention locally.

![Tab2 global](data/src/README/Tab2_entry_global.png)
![Tab2 local](data/src/README/Tab2_entry_local.png)

### Tab3 window: view energy

Press `View Info` button from main window, you'll get into Tab3 window.
This function is to check energy distribution of a bin file in detail.
By different level of selection, you can view energy curves of GMSL/STiC/channel sets.

![Tab3 global](data/src/README/Tab3_energy_global.png)
![Tab3 local](data/src/README/Tab3_energy_local.png)

### Tab4 window: reconstruction & visualize

Press `View Image` button from main window, you'll get into Tab4 window.
In this function, you can adopt either FBP or ML-EM algorithms to reconstruct image.
> Since the recon geometry depends on board topology,
> there is a section to adjust channel arrangement on top left side.

![Tab4 window](data/src/README/Tab4_window.png)

1. Press `FBP recon` button to start FBP functoin, and show recon result.
  In this function, it also process essential materials for ML-EM, i.e. `data/recon/LOR/<bin>/LOR_ZY_<bin>.txt`.
  After finishing the program, the FBP results would show on the image panel.
  ![Tab4 FBP recon](data/src/README/Tab4_FBP_recon.png)

2. Press `SRM gen` button to generate SRM files.
  First, set some arguments (xplanes & spacing) in config panel. Then the program will run on background.
  (Note: This function takes much longer time (maybe few days) depending on the capability of device.)
  ![Tab4 SRM config](data/src/README/Tab4_SRM_config.png)

3. Press `ML-EM recon` button to run ML-EM reconstruction.
  First, set some arguments (iterations, spacing, voxel-x & xplanes) in config panel.
  ![Tab4 MLEM config](data/src/README/Tab4_MLEMrecon_config.png)
  The output files would be saved as `MLEM_<num>ite_<bin>.DAT` in `data/recon/MLEM/OUTPUT/<bin>/`.
  ![Tab4 MLEM 10ite files](data/src/README/Tab4_MLEMrecon_file.png)
  The running progress is also displayed in terminal.
  ![Tab4 MLEM terminal output](data/src/README/Tab4_MLEMrecon_terminal.png)

4. Press `ML-EM vis` button to select an MLEM output DAT to visualize.
  Since MLEM recon takes too much time, the visualization part is separately constructed,
  so that users won't go through whole MLEM recon progress every time.
  Just choose which MLEM DAT file to visualize, then view output images slice by slice.
  ![Tab4 MLEM vis](data/src/README/Tab4_MLEMvis_Derenzo.png)

5. Press `3-angle recon` button to run 3-angle MLEM reconstruction.
  First, set some arguments (iterations, spacing, voxel-x & xplanes) in config panel.
  ![Tab4 3-angle config](data/src/README/Tab4_3angleRecon_config.png)

6. Press `3-angle vis` button to select 3-angle MLEM output DATs to visualize.
  Just choose which 3-angle MLEM DAT outputs to visualize.
  ![Tab4 3-angle vis](data/src/README/Tab4_3angleVis_config.png)
  Then view images slice by slice with different acquisitions.
  ![Tab4 3-angle vis](data/src/README/Tab4_3angleVis_Derenzo_aq2.png)

> [Hint] If you want to run MLEM, 3-angle or SRM generation by independent exe file,
> please reference to [README_cmd_snippet.md](data/src/README/setup/README_cmd_snippet.md)

------------------------------------------------------------------------------

## Working progress

Current progress of software development.
Take notes down for reminding:

- [ ] Gate data: Integrate OpenGATE simulation data for reconstruction.
- [ ] MLME vis: Add axis info on the image.
- [ ] Dynamically adjust arguments of option with buffer (using new config.json).

## Issues

Recent issues and fixing dialog of development.

- [X] MLEM recon: Uptimize ML-EM program.
  > - Separate LOR sorting part from FBP recon. (For 3-angle recon)
- [X] 3-angle recon: Integrate preprocessing steps for 3-angle recon.
- [X] Thread: Fix issue of testing sample with OMP.

## Future work

Recomendation of future work.

- [ ] MLEM vis: Introduce PyQt5 3D visualization module for MLEM.
- [ ] Tab4: Modify image panel with scalable QGraphicsView().
- [ ] Threading function: Apply OpenMP to SRM program.
