from pyexpat import version_info
import re
from tkinter.tix import MAIN
import shutil

class version_class:
    def __init__(self, path=''):
        self.scpu_version = [0, 0, 0, 0]
        self.ncpu_version = [0, 0, 0, 0]
        self.model_version = [0, 0, 0, 0]
        self.scpu_date = ''
        self.ncpu_date = ''
        self.model_date = ''
        self.path = path

        if path != '':
            self.get_version_info(path=path)
        pass

    def print_values(self):
        print('self.scpu_version:', self.scpu_version)
        print('self.scpu_date:', self.scpu_date)
        print('self.ncpu_version:', self.ncpu_version)
        print('self.ncpu_date:', self.ncpu_date)
        print('self.model_version:', self.model_version)
        print('self.model_date:', self.model_date)

    def get_version_info(self, path):
        ver_dict = dict()
        self.path = path
        with open(path, "r") as f:
            while True:
                data = f.readline()
                # data_array = data.split('\w')
                data_array = re.split('(?:[= \n\r])', data)

                if not data:
                    break

                ver_dict[data_array[0]] = data_array[1]

        # print(ver_dict)

        self.scpu_version = [int(ver_dict['SCPU_VERSION[0]']), int(ver_dict['SCPU_VERSION[1]']), int(ver_dict['SCPU_VERSION[2]']), int(ver_dict['SCPU_VERSION[3]'])]
        self.ncpu_version = [int(ver_dict['NCPU_VERSION[0]']), int(ver_dict['NCPU_VERSION[1]']), int(ver_dict['NCPU_VERSION[2]']), int(ver_dict['NCPU_VERSION[3]'])]
        self.model_version = [int(ver_dict['MODEL_VERSION[0]']), int(ver_dict['MODEL_VERSION[1]']), int(ver_dict['MODEL_VERSION[2]']), int(ver_dict['MODEL_VERSION[3]'])]
        self.scpu_date = ver_dict['SCPU_VER_DATE']
        self.ncpu_date = ver_dict['NCPU_VER_DATE']
        self.model_date = ver_dict['MODEL_VER_DATE']

    def update_scpu_version(self, path):
        file_data = ''
        with open(path, "r") as f:
            while True:
                data = f.readline()
                if '#define' in data and 'SCPU_VERSION_DATE' in data:
                    data = '#define SCPU_VERSION_DATE    (%s)\n'%self.scpu_date
                elif '#define' in data and 'SCPU_VERSION0' in data:
                    data = '#define SCPU_VERSION0       (%d)\n'%self.scpu_version[0]
                elif '#define' in data and 'SCPU_VERSION1' in data:
                    data = '#define SCPU_VERSION1       (%d)\n'%self.scpu_version[1]
                elif '#define' in data and 'SCPU_VERSION2' in data:
                    data = '#define SCPU_VERSION2       (%d)\n'%self.scpu_version[2]
                elif '#define' in data and 'SCPU_VERSION3' in data and 'SCPU_VERSION3_REAL' not in data:
                    data = '#define SCPU_VERSION3       (%d)\n'%self.scpu_version[3]

                if not data:
                    break
                file_data += data

        # print(file_data)

        with open(path, "w") as f:
            f.write(file_data)

    def update_model_version(self, path):
        file_data = ''
        with open(path, "r") as f:
            while True:
                data = f.readline()
                if '#define' in data and 'MODEL_VERSION_DATE' in data:
                    data = '#define MODEL_VERSION_DATE   (%s)\n'%self.model_date
                elif '#define' in data and 'MODEL_VERSION0' in data:
                    data = '#define MODEL_VERSION0       (%d)\n'%self.model_version[0]
                elif '#define' in data and 'MODEL_VERSION1' in data:
                    data = '#define MODEL_VERSION1       (%d)\n'%self.model_version[1]
                elif '#define' in data and 'MODEL_VERSION2' in data:
                    data = '#define MODEL_VERSION2       (%d)\n'%self.model_version[2]
                elif '#define' in data and 'MODEL_VERSION3' in data:
                    data = '#define MODEL_VERSION3       (%d)\n'%self.model_version[3]

                if not data:
                    break
                file_data += data

        # print(file_data)

        with open(path, "w") as f:
            f.write(file_data)

    def update_ncpu_version(self, path):
        file_data = ''
        with open(path, "r") as f:
            while True:
                data = f.readline()
                if '.date' in data:
                    data = '    .date = %s,\n'%self.ncpu_date
                elif '.version[0]' in data:
                    data = '    .version[0] = %d,\n'%self.ncpu_version[0]
                elif '.version[1]' in data:
                    data = '    .version[1] = %d,\n'%self.ncpu_version[1]
                elif '.version[2]' in data:
                    data = '    .version[2] = %d,\n'%self.ncpu_version[2]
                elif '.version[3]' in data:
                    data = '    .version[3] = %d,\n'%self.ncpu_version[3]

                if not data:
                    break
                file_data += data

        # print(file_data)

        with open(path, "w") as f:
            f.write(file_data)

    def backup_version(self, path):
        shutil.copy2(self.path, path)
        pass

if __name__ == "__main__":
    version_info = version_class()
    version_info.get_version_info('./version.cfg')
    version_info.print_values()
    version_info.update_scpu_version('./version.c')
    version_info.update_ncpu_version('./version.h')