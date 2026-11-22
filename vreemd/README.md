# Code van anderen

Deze map speelt de rol van code van anderen: een bibliotheek die een project overneemt, met de
fouten die zulke code vaak heeft. De fouten staan er met opzet in. Repareer ze niet.

| Map | Rol |
| --- | --- |
| `minilib/` | Een kleine bibliotheek in C-stijl, met geplante fouten |

## Hoe de bouw de code behandelt

| Preset | Headers van `minilib` | Waarschuwingen op `minilib` |
| --- | --- | --- |
| alle gewone presets | `-isystem`, zoals CMake doet bij een geïmporteerde bibliotheek | geen |
| `clang-audit` | gewone `-I` | alle waarschuwingen, plus `-Wunsafe-buffer-usage` |

In de presets met ASan bouwt `minilib` twee keer:

| Doel | Wat het nabootst |
| --- | --- |
| `minilib` | De bibliotheek is uit de bron gebouwd, met de sanitizer van de preset |
| `minilib_prebuilt` | Een binair pakket, bijvoorbeeld uit conda: altijd zonder sanitizer gebouwd |

## Geplante fouten

| ID | Plaats | Fout |
| --- | --- | --- |
| V1 | `minilib.hpp`, `ml_sum` | Teller niet geïnitialiseerd |
| V2 | `minilib.hpp`, `ml_sum` | `<=` in plaats van `<`: leest één element te veel |
| V3 | `minilib.hpp`, `ml_count` | Stille versmalling van `long` naar `unsigned` |
| V4 | `minilib.cpp`, `ml_get` | Geen grenscontrole op de index |

Het experiment [e003](../experimenten/e003-code-van-anderen/README.md) meet welk vangnet welke
fout vindt.
