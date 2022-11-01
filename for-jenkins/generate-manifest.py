import pandas as pd
import numpy as np
from io import StringIO
import re,sys,getopt

sl7_only_packages=('caendigitizer','pqxx','epics','jsoncpp','redis','zmq','caencomm','caendigitizer','caenvme')

class ProductDepsReader:
    def __init__(self, name):
        self.name=name


    def read(self, header,trailer, usecols=None):
        lines=StringIO()
        with open(self.name) as input_data:
            for line in input_data:
                if line.strip().startswith(header):
                    break
            lines.write(line)

            for line in input_data:
                if line.strip().startswith(trailer):
                    break
                lines.write(line)

        lines.seek(0)

        return pd.read_csv(lines, delimiter=r"\s+",usecols=usecols)


    def read_qualifiers(self):
        return self.read('qualifier','end_qualifier_list')


    def read_products(self):
        return self.read('product','end_product_list',range(3))


class Product:
    def __init__(self,product,qualifiers):
        self.product=product
        self.qualifiers=qualifiers

    def manifest_entry(self):
        name=self.product[0]
        version=self.product[1]
        optional=self.product[2]

        qualifiers=self.qualifiers.split(':')
        rqual=list(filter(re.compile(r'(e[0-9]{2}|c[0-9]{1})').search , qualifiers))
        bqual=list(filter(re.compile(r'(debug|prof)').search , qualifiers))
        squal=list(filter(re.compile(r's[0-9]{2,3}').search , qualifiers))
        pyqual=list(filter(re.compile(r'(py(2|3)|p2715a|p383b|p392)').search , qualifiers))
        aqual=list(filter(re.compile(r'(noarch|slf7-x86_64)').search , qualifiers))

        arch='slf7-x86_64' if not aqual else aqual[0]

        if name in sl7_only_packages and arch.startswith('slf'):
            arch=arch.replace('slf','sl')

        tarball='{0:s}-{1:s}-{2:s}'.format(name,version.replace('_', '.').replace('v','') ,arch)

        if rqual:
            tarball += '-{0:s}'.format(rqual[0])
        if pyqual:
            tarball += '-{0:s}'.format(pyqual[0])
        if squal:
            tarball += '-{0:s}'.format(squal[0])
        if bqual:
            tarball += '-{0:s}'.format(bqual[0])

        tarball +='.tar.bz2'
        entry = '{0:20s} {1:10s} {2:70s}'.format(name,version,tarball)
        entry += ' -f {0:20s}'.format('Linux64bit+3.10-2.17' if arch == 'slf7-x86_64' else 'NULL')

        if self.qualifiers !='-nq-' and  self.qualifiers != '-' and self.qualifiers != 'noarch':
            entry += ' -q {0:20s}'.format(self.qualifiers)

        entry += '\n'

        return entry



class ManifestBuilder:
    def __init__(self,name,qualifiers):
        self.name=name
        self.qualifiers=qualifiers
        self.df_products=ProductDepsReader(self.name).read_products()
        self.df_qualifiers=ProductDepsReader(self.name).read_qualifiers()
        self.manifest=StringIO()


    def match_product_qualifiers(self):
        for index, row in self.df_qualifiers.iterrows():
           if  row[0] == self.qualifiers:
               return row

           candidate=row[0].split(':')
           reference=self.qualifiers.split(':')
           if len(candidate) != len(reference):
               continue

           if set(candidate) == set(reference):
               return row


    def build_manifest(self):
        manifest=StringIO()
        product_qualifiers=self.match_product_qualifiers()

        for index,product in self.df_products.iterrows():
            qualifiers='noarch' if product[0] not in product_qualifiers else product_qualifiers[product[0]]

            if product[2] not in [np.nan,'-'] and not all(q in product_qualifiers['qualifier'].split(':') for q in product[2].split(':')):
                    continue

            product=Product(product,qualifiers)
            manifest.write(product.manifest_entry())

        manifest.seek(0)

        return manifest


def main(argv):
    product_deps_file = 'product_deps'
    manifest_file = 'manifest.txt'
    qualifiers=None

    try:
        opts, args = getopt.getopt(argv,"hp:m:q:",["product_deps=","manifest=","qualifiers="])
    except getopt.GetoptError:
        print('generate-manifest.py -p <product_deps> -p <manifest> -q <qualifiers>')
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print('generate-manifest.py -p <product_deps> -m <manifest> -q <qualifiers>')
            sys.exit()
        elif opt in ("-p", "--product_deps"):
            product_deps_file = arg
        elif opt in ("-m", "--manifest"):
            manifest_file = arg
        elif opt in ("-q", "--qualifiers"):
            qualifiers = arg

    builder = ManifestBuilder(product_deps_file,qualifiers)
    manifest = builder.build_manifest()

    with open(manifest_file, mode='w') as f:
        print(manifest.getvalue().strip(), file=f)

if __name__ == "__main__":
    main(sys.argv[1:])




