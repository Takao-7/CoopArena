# How-To: Pull request

## Vorbereitung:
1. Unreal Engine und Visual Studio **schließen**
2. Github Desktop öffnen

## Ablauf
1. Änderungen committen
2. (GitHub Desktop) In der Leiste oben auf *Branch* -> Update from Default Branch klicken
3. Falls Konflikte entdeckt wurden löst diese. Falls nicht, glück gehabt.
4. Nachdem die Konflikte gelöst wurden, erstellt einen pull request:
    1. Branch -> Create pull request
    2. Github.com öffnet sich. Schreibt in den Titel was ihr mit dem pull request hinzugefügt habt.

## Konflikte lösen
1. Visual Studio öffnen
2. Unten rechts (blaue Leiste) auf den Bleistift klicken
3. Es erscheint (vermutlich rechts) ein neues Fenster (Team Explorer). Dort auf *Conflicts:* klicken  
Dort werden alle in konflikt stehenden Dateien aufgelistet. Handelt es sich um eine Code (C++ / Header) Datei, werden alle in Konflikt stehenden Änderungen nebeneinander aufgelistet, wenn man auf *Compare Files* klickt.
4. Bei **Code konflikten** müsst ihr dann entscheiden welche Code Abschnitte übernommen werden sollen.
5. Bei anderen **Dateikonflikten** (z.B. usassets) könnt ihr natürlich den Code nicht vergleichen, sondern müsst entscheiden welche der beiden Dateiversionen die Richtige ist.
    - Habt ihr diese Datei **explizit geändert** und ist diese **für euren Commit relevant**?  
  -> Falls **ja** hat eure Änderung vorrang (*Keep Target*)  
  -> Falls **nein**, hat die Quelle vorrang (*Keep Source*)