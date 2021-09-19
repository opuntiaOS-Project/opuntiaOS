import requests
import lxml.html as lh
import pandas as pd
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('arch', type=str, help='Arch [arm, x86]')
args = parser.parse_args()

url='https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md'
page = requests.get(url)
doc = lh.fromstring(page.content)
tr_elements = doc.xpath('//tr')

start = 0

for i, tr in enumerate(tr_elements):
    if tr[0].text_content() == "syscall name":
        start = i + 1


data = []
for tr in tr_elements[start:]:
    table_col = 2
    if args.arch == "x86":
        table_col = 4
    if tr[table_col].text_content() != "-":
        data.append([tr[0].text_content(), int(tr[table_col].text_content())])

data.sort(key=lambda x: x[1])

for i in data:
    print("SYS_{0} = {1},".format(i[0].upper(), i[1]))