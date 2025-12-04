
## Aufgabenstellung
Ziel: Implementierung der Multiplikation dünnbesetzter Matrizen im Ellpack (column-major) Format.

## Lösungsansätze
Das Lesen im column-major Format ist cache-ineffizient. Deshalb wurde ein `Row-Cache` implementiert, welcher jeweils eine Zeile aus der Matrix (column-major) herauszieht. Diese wird anschließend mit allen Spalten der zweiten Matrix multipliziert.

Um die Eingabematrix strukturierter zu verarbeiten, wurde das Struct ELLPACKMatrix definiert. Dies enthält u.a. wie viele gültige Werte in den einzelnen Spalten sind.

Die Ergebnisse werden in `result_mat` zwischengespeichert. Dies besteht aus einem Array von dynamischen Arrays `result_col` (Zeilenanzahl vorher unbekannt). Schreiben der Arrays (amortisiert O(1)) kann einen cache-miss verursachen, dafür ist Lesen so effizienter.

## Projektstruktur
`dev`: Dateien zur Generierung (Benchmarking, Tests)
`include`: Definition von Datenstrukturen, die wenige Berechnungen mit sich, bringen aber von vielen anderen Dateien genutzt werden
`src`: Dateien, welche für die Implementierung der Projektspezifikationen benötigt werden

## Messumgebung
- Prozessor: Ryzen 5 4500U; Frequenz: ca. 1,4 GHz
- Arbeitsspeicher 7,4 GB
- Betriebssystem: Ubuntu 24.04 LTS
- Kernelversion: 6.8.0-38-generic
- Compiler: gcc 13.2.0
- Turboboost option und Hyperthreading deaktiviert

## Benchmarking
Wir haben drei Implementierungen verglichen: Algorithmus mit und ohne Annahme, dass Indizes und Werte in Ellpack nach
aufsteigendem Spaltenindex sortiert sind. SIMD-Arithmetik für Multiplikation und
Addition des Skalarprodukts.
Es kam heraus, dass eine deutliche Performanzsteigerung mit SIMD erzielt werden kann. 
Die Logikoptimierung den row cache nicht immer neu zu durchsuchen war ebenfalls wirksam.
Faktoren der Optimierung stiegen mit der Größe der Matrizen.

## eigener Anteil

Nach einer kurzen Phase der allgemeinen Informationssammlung, wurden die Aufgaben in die drei Bereiche Benchmark Setup, Test Setup, und Ein- und Ausgabe unterteilt.

## Artem
Organisation der Projektstruktur, Implementierung des Benchmarkings und Implementierung eines Großteils des Kernalgorithmus. Am interessantesten fand ich die Implementierung des Row-Caches, und die darauf folgende Optimierung, mit der dieser mit nur zwei Zeilendurchläufen immer die nächste Zeile angibt. Das Implementieren des Benchmarking-Generators hat mir dabei besonders geholfen, das Problem gut zu verstehen und über mögliche Performanzrandfälle nachzudenken.

## Simon
Automatisierte Generation von zufälligen Matrizen in dense-Format und deren Produkt mit trivialer Multiplikation aus
Vorlesung. Automatisierte Test Funktion, die Matrizen generiert und das Ergebnis der Ellpack Multiplikation vergleicht.
Auswertung der Benchmark-Ergebnisse.


## Marina
Implementierung der I/O Funktionalität (io.c & io.h), der Ein- und Ausgabedatenstrukturen `ELLPACKMatrix`, `result_mat` und des dynamischen Arrays `result_col`. Sowie die Umsetzung des Projektberichts. Bei den Eingabematrizen wurde u.a. darauf getestet, dass die * an den gleichen Indices für Wert und Index sind und die richtige Anzahl an Werten bzw. Indices übergeben wird.

