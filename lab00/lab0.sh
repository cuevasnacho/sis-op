#ejercicio1
cat /proc/cpuinfo | grep "model name" | head -n 1

#ejercicio2
grep "model name" /proc/cpuinfo | wc -l

#ejercicio3
wget -O - https://www.gutenberg.org/files/11/11-0.txt | sed 's/Alice/Ignacio/g' > IGNACIO_in_wonderland.txt

#ejercicio4
sort -n -k5 -r weather_cordoba.in > weather_cordoba_sorted.in && cat weather_cordoba_sorted.in | head -n 1 | cut -c-9 && cat weather_cordoba_sorted.in | tail -n 1 | cut -c-9 && rm weather_cordoba_sorted.in

#ejercicio5
sort -n -k3 atpplayers.in

#ejercicio6
awk '{print $1 " " $2 " " $3 " " $4 " " $5 " " $6 " " $7 " " $8 " " $7-$8}' superliga.in | sort -n -k2 -k9 -r | awk '{print $1 " " $2 " " $3 " " $4 " " $5 " " $6 " " $7 " " $8}'

#ejercicio7
ip address | grep link/ether | head -n 1 | awk '{print $2}'

#ejercicio8
mkdir serie && touch serie/fma_S01E{01..10}_es.srt
for i in *.srt; do newname=`echo $i | sed 's/_es//g'`; mv $i $newname; done

#ejercicio9a
ffmpeg -i video.mkv -ss 00:00:04.0 -t 00:00:04.0 recortado.mvk

#ejercicio9b
ffmpeg -i audio_principal.mp3 -i musica.mp3 -filter_complex amix=inputs=2 audio_con_musica.mp3

