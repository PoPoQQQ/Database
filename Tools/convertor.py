import pandas as pd
import argparse

# parser
parser = argparse.ArgumentParser()
parser.add_argument("table", help="The table to insert data", type=str)
parser.add_argument("-c", "--csv", help="Load csv file from given path")

def tbl2sql(filename, tablename):
    with open(filename, 'r') as file:
        df = pd.read_table(file, '|', header=None)
        print(len(df.columns))
        types = []
        for i in range(len(df.columns) - 1):
            types.append(input("input data type(INT = 1, VARCHAR = 2, DATE = 3, FLOAT = 4): "))
        with open('output.sql', 'a+') as out:
            for index, row in df.iterrows():
                clause = "( "
                for i in range(len(row) - 1):
                    if types[i] == '1':
                        clause += str(row[i])
                    elif types[i] == '2':
                        clause += "'%s'" % row[i]
                    elif types[i] == '3':
                        clause += "'%s'" % row[i]
                    elif types[i] == '4':
                        clause += str(row[i])
                    if i < len(row) - 2:
                        clause += ", "
                    else:
                        clause += ")"
                out.write('INSERT INTO ' + tablename + ' VALUES ' + clause + ';\n')

if __name__ == '__main__':
    args = parser.parse_args()
    tbl2sql('part.tbl', args.table)