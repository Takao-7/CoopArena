# CoopArena
## Projektbeschreibung
- Ziel der Spieler / des Spielers ist es in einem Match eine bestimmte Zeit, ca. 15 - 20 Minuten, zu überleben und dabei eine möglichst Hohe Punktzahl zu erreichen.
Punkte werden vergeben für:
    - Gegner auszuschalten
    - Unentdeckt bleiben
    - Gegner unentdeckt auszuschalten
    - Überleben: Je länger man überlebt, desto mehr Punkte
    - Keine Mitspieler verlieren
- Singleplayer und Coop (bis zu 4-6 Personen) sind möglich
- Anzahl der Gegner richtet sich nach der Anzahl der Spieler
    - Optional: Möglichkeit auf Battle Royal Modus, wo mehrere Spieler / Teams gegeneinander antreten, während sich versuchen zu überleben.
- Gespielt wird aus der 1. Person (Ego-Shooter) oder 3. Person.  
Dies kann zu einem späteren Zeitpunkt entschieden werden, da die Wahl hier nicht das Grundkonzept des Spiels beeinflusst.
- Spawnpunkte der Gegner sind zufällig, ebenso der Spawnpunkt der Spieler
- Optional: Zufallsgenerierte Karte
    - Hierzu einen Algorithmus zu schreiben könnte aufwendig sein. Objekte einfach zufällig im Level zu Spawnen ist nicht das Problem. Diese in Sinnvoll anzuordnen, so dass die Karte nicht „zusammengewürfelt“ aussieht ist das Problem.
    - Es gibt ein Beispielprojekt, inkl. Video von Epic Games, wo ein Dungeon mit hilfe eines Algorithmus generiert wird. Dieser könnte evtl. adaptiert werden.
- Eine Möglichkeit zur Balance und zur Erhöhung des Wiederspielwertes wäre es eine Art Shopsystem einzuführen:
    - Sämtliche Ausrüstung und Munition kostet Geld. Wenn man stirbt, verliert man die gesamte Ausrüstung.
    - Für verdiente Punkte während des Matches (siehe oben) erhält man Geld. Außerdem kann man alle gefundenen Gegenstände und Waffen behalten und daher auch später wieder nutzen.
    - Dieses System würde es außerdem einfacher machen verschiedene Klasse und Ausrüstung zu balancieren, weil diese dann nicht alle gleich gut sein müssen: Bessere Klassen (bzw. deren Ausrüstung) kostet einfach mehr Geld.

## Style und Spielhintergrund

## Nutzbare Assets
Epic Games stellt sehr viele Assets frei zur Verfügung, u.a.:
- Ein „Shooter Game“ Beispielproject mit einem futuristischen Soldaten und zwei Waffen
    - Hiervon sind insbesondere das Sturmgewehr und der Soldat sehr interessant.
    - Der Code kann auch teilweise genutzt werden bzw. als Grundlage genutzt werden
- Seit neuestem: Die Charaktere aus dem Spiel „Paragon“
    - Das Setting von Paragon ist Sci-Fantasy. Die meisten Charaktere besitzen Nahkampf- und keine Fernkampfwaffen, daher wäre eine sinnvolle Implementierung schwierig.
    - ODER: Man setzt das Setting komplett auf Paragon und anstelle von einer Soldatenklasse wählen die Spieler je einen der Charaktere.
- Mehrere Gelände Assets Packs, unter anderem:
    - Open World: Enthält Bäume, Vegetation, Felsen, etc.
    - Sci-Fi Corridor Pack & Bunk Bed Scene: Enthalten Sci-Fiction Interiour Assets.
- Animationen Pack

Es wird also nicht notwendig sein, Assets komplett selbst zu machen. Anstelle kann man sich mehr auf die Programmierung der Gameplayfeatures konzentrieren.

Für eine nicht-zufallsgenerierte Karte wäre eine Offene Landschaft mit Wäldern und Ruinen vermutlich die beste Wahl, für eine Zufallsgenerierte Karte ein Gebäude / Basis, etc.

## To-Do Liste Softwarepraktikum

- Versionsverwaltung Github 
- waffle.io
- Discord

### Bereiche
- Leveldesign
    - Größe?
- KI
- User Interface
- Pickups
    - UI für das Aufheben
- Waffen
    - Soundeffekte
- Fragmentierung von Meshes
- Spielelogik: Wie läuft eine Runde ab?
- Spiel multiplayerfähig machen

### Offene Fragen
- Wie viele Gegnertypen?
    - Nah-, Fernkampf oder beides?
- Was für Gegner und was für ein Gegnerstyle? Z.B. nur Maschinen oder nicht-Menschen.
- Verschiedene Munitionstypen?
- Inventar System?
- Paragon Charaktere?
- Welche zusätzlichen Animationen sind noch notwendig?  
Kriechanimationen und ein Nahkampfangriff sind nicht im *Starter Animation Pack* vorhanden.  
=> Adobe's *Mixamo* Webservice hat sehr viele kostenlose Animationen.

### Features 1. Version
- Singleplayer
- 1 Spielerklasse
- 1 Gegnerklasse
- 1 Karte
- 1 Waffe

### Weitere Features
- Inverse Kinematik für Füße und Beine  
Sorgt dafür, dass der Spieler auf unebenen Boden "korrekt" steht und nicht halb in der Luft schwebt
- Korrekte Rotationsanimationen  
Wenn man sich auf der Stelle dreht, sollte sich der Charakter nicht einfach wie auf einer Drehscheibe drehen, sondern seinen Körper korrekt animiert drehen.
- Squad Befehle  
Möglichkeit seinen Teammitglieder Bewegungs- oder Angriffsbefehle zu geben, bzw. sichtbare Marker zu platzieren
- In-Game Voicechat