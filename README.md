# Servizi Video Multicast TIM
Repository contenente scripts per vedere i contenuti trasmessi in rete FTTx TIM tramite tecnologia Multicast
---
## Gruppi Conosciuti (porta 5004)
| Indirizzo Gruppo | Contenuto | File |
| ---------------- | --------- | ---- |
| `239.200.0.0`    | Contenuti Disponibili | `live_catalog.csv` |
| `239.200.0.2`    | Trasmissioni HLS | `index.m3u8` + Segmenti (HLS) |
| `239.200.0.80`   | Canale 5 | Manifest + Segmenti (MPEG-DASH) | 
---
## Utilizzo
Ottenere Lista Flussi
- `./mc-server-emu 239.200.0.0 5004` fino all'ottenimento di 2/3 pacchetti dopo il `live_catalog.csv`
- `python infos_extractor.py live_catalog.csv`
- ottenimento file decoded_data.json

Visualizzare video HLS in diretta:
- `./mchls2ts 239.200.0.2 5004 | vlc -`

