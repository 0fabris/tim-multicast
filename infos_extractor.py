import re
import sys
from ipaddress import ip_address
from urllib.parse import urlencode, parse_qsl
import json

#info parser

class MulticastStream:
    def __init__(self):
        self.url = ''
        self.multicastrow = ''

    def fromDataRow(self, datarow):
        sep = datarow.find(";")
        self.url = datarow[:sep]
        self.multicastrow = datarow[sep+1:]
        self.multicastVideoStreams = dict()
        self.serviceType = ''
        self.serviceID = ''
        self.rfun = ''
        self.dataSpeed = ''
        self.multicastAudioStreams = dict()

        #parse row
        self.parseMulticastRow()
        

    def _getRowDict(self):
        row = dict()
        params = self.multicastrow.split("?")[-1].split("&")
        
        for p in params:
            pos = p.find('=')
            row[p[:pos]] = p[pos+1:]
        return row

    def parseMulticastRow(self):
         
        row = self._getRowDict()
        self.serviceType = row['st'] if 'st' in row.keys() else 'unknown'
        self.serviceID = row['sri'] if 'sri' in row.keys() else 'unknown'
        self.rfun = row['rfun'] if 'rfun' in row.keys() else 'unknown'
        
        self.dataSpeed = row['dspd'] if 'dspd' in row.keys() else 'unknown'
        
        if 'lsv' in row.keys():
            for video in row['lsv'].split(";"):
                self.multicastVideoStreams['{}:{}'.format(ip_address(row['mi'])+len(self.multicastVideoStreams),row['mp'])] = video[2:]
        
        if 'pla' in row.keys():
            for audio in row['pla'].split(";"):
                self.multicastAudioStreams['{}:{}'.format(ip_address(row['mia'])+len(self.multicastAudioStreams),row['mpa'])] = audio[2:]
        if 'lsa' in row.keys():
            for audio in row['lsa'].split(";"):
                self.multicastAudioStreams['{}:{}'.format(ip_address(row['mia'])+len(self.multicastAudioStreams),row['mpa'])] = audio[2:]
        if 'plaa' in row.keys():
            for audio in row['plaa'].split(";"):
                self.multicastAudioStreams['{}:{}'.format(ip_address(row['mia'])+len(self.multicastAudioStreams),row['mpa'])] = audio[2:]
        if 'lma' in row.keys():
            for audio in row['lma'].split(";"):
                self.multicastAudioStreams['{}:{}'.format(ip_address(row['mia'])+len(self.multicastAudioStreams),row['mpa'])] = audio[2:]

    def toJSON(self):
        return {
            'url': self.url,
            'mcDetails': self._getRowDict(),
            'mcVideoStreams': self.multicastVideoStreams,
            'mcAudioStreams': self.multicastAudioStreams,
            'mcServiceType': self.serviceType,
            'mcServiceID': self.serviceID,
            'mcRFUN': self.rfun,
            'mcDataSpeed': self.dataSpeed
        }

def toHexView(arr):
    return " ".join(format(x,"02x") for x in arr)

def decodeData(content):
    streams = []
    rows = content.split(b"\n")[:-1]
    for row in rows:
        row = row.decode()
        # parse url and write
        r = MulticastStream()
        r.fromDataRow(row)
        streams.append(r.toJSON())
    with open("decoded_data.json",'wb') as f:
        f.write(json.dumps(streams, indent=4).encode())

if __name__ == "__main__":

    if len(sys.argv) < 2:
        exit("usage: "+sys.argv[0] + " fname")

    fname = sys.argv[1]

    with open(fname,"rb") as f:
        bytesarr = f.read()
        decodeData(bytesarr)


