# Analisi Pacchetti
## Header
- [0:2] Header Type
- [2:4] Contatore
- [4:7] Altro Contatore (last byte is header[3] - 4)?
- [7] sconosciuto
- [8:10] Stream Type
    - `90 xx` = Contenuto ?
    - `ef xx` = Metadati ?
- [10:13] Contatore "Timer"?
- [14] Contatore File inviati
- byte[15] == `00` -> Relativo all'ultimo Segment ?
    - udpheader[17:20] Indirizzo di partenza della scrittura sul file 
- byte[15] == `01` -> Relativo all'init Segment ?
    - udpheader[16:18] Indirizzo di partenza della scrittura sul file ?
- byte[15] == `02` -> Relativo al manifest
    - udpheader[16:18] Indirizzo di partenza della scrittura sul file ?
---
## `80 21` "Data" Headers
Esempio `80 21 5c 8a 03 28 1e 30 90 00 46 05 01 03 d5 00 00 0d 3c 18`

- Lunghezza fissa a `20 bytes`
- udpheader[16:18] != `00 00` && udpheader[18:20] == `00 00`
    - content = HTTP Headers
            
- udpheader[13] == `02` -> Nuovo File 
- udpheader[13] == `03` -> Continuo Ricezione File
- udpheader[13] == `05` -> HTTP request body
---
## `80 A1` "Control" Headers
Esempio `80 a1 5c 89 03 28 1d 6a 90 00 46 05 01 02 d5 00`

- Lunghezza fissa a `16 bytes`
- udpheader[13] == `01` && udpheader[15] == `00` -> Nuovo Segmento
- udpheader[13] == `01` && udpheader[15] == `01` -> Segmento Init (DASH)
- udpheader[13] == `01` && udpheader[15] == `02` -> Riscrivo Manifest, byte[14] + `10h`
- udpheader[13] == `02` -> Refresh del nome  del manifest
Il 'filename' ha lunghezza massima 300 + 1 (termina stringa `\0`)
