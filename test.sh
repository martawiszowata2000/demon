!/bin/bash
echo "Tworzenie katalogu zrodlowego i docelowego"
mkdir ~/source ~/destination

echo "Tworzene drzewa plikow do synchronizacji"
mkdir ~/source/a ~/source/b ~/source/b/bb ~/source/c
touch ~/source/jedenplik ~/source/a/drugiplik ~/source/b/trzeciplik

echo "Uruchamiam demona, z synchronizacją rekursywną co 10 sekund"
demon -R -s 10 ~/source ~/destination

echo "Porownuję katalog zrodlowy z docelowym: "
diff ~/source ~/destination
