# Algorithm Engineering Exercise

## Input
- 64-bit integers, entweder long long oder int64_t nutzen
- Zufällig gemischt
- befinden sich in einem Array, welcher Pointer beinhaltet
- jeder Pointer zeigt auf einen Block mit den Integern

## Output
- Array mit Pointern, die auf Blöcke von sortierten Integern zeigen

## Algorithmus
MSD Radix Sort sortiert eine Menge an Integern nach der ersten/höchsten Ziffer jeder Zahl

Algorithmus sortiert Integer, indem die einzelnen Bits durchgegangen werden und danach jeweils Container 0 und 1 erstellt werden. In diese Container werden dann die Integer jeweils einsortiert.

Damit der Algorithmus In-Place bleibt, befindet sich der Bin für 0en am Anfang, für 1en am Ende.

Das erste Element des Arrays wird beim höchsten Bit angesehen.
- Ist das Bit eine 1: Tausche es mit dem Element vor der inneren Grenze des 1-Bins, dekrementiere die Grenze des 1-Bins
- Ist das Bit eine 0: Tausche es mit dem Element nach der inneren Grenze des 0-Bins, inkrementiere die Grenze des 0-Bins

Das nächste Element, welches betrachtet werden soll, ist das erste Element nach dem 0-Bin. Dies wird so lange ausgeführt, bis die Grenzen des 1- und 0-Bins sich treffen.

Daraufhin wird rekursiv für die beiden Bins das Sortieren erneut für das nächste Bit durchgeführt.

Sobald die Bins klein genug werden, verwende Robin Hood Sorting:\
Zunächst wird ein Buffer, welcher z.B. 2.5 mal die Größe des zu sortierenden Bins besitzt, allokiert. Daraufhin werden die Elemente an die jeweilige Position im Buffer geschrieben. Die Position ergibt sich, indem der minimale Wert in diesem Bin, welcher möglich wäre, vom aktuellen Element abgezogen wird und schließlich ein Bitshift ausgeführt wird. Geshiftet wird so, dass alle bisher betrachteten Bits verloren gehen.\
Falls kein Platz an dieser Position existiert, platziere das Element hinter allen Elementen, welche kleiner oder gleich groß sind und bei bzw. nach dieser Position liegen. Dies wird auch als Robin Hood Hashing bezeichnet.\
Schließlich werden die Elemente zurück in den jeweiligen Bin geschrieben, wobei alle leeren Einträge des Buffers ignoriert werden.
