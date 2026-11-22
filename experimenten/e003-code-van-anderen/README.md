# e003 — Code van anderen: verborgen waarschuwingen en blinde vlekken

**Catalogus:** G24, G25, G26 · **Status:** onderzocht · **Datum:** 2026-11-22

## Vraag

Een project neemt een bibliotheek van anderen over. Welke fouten in die bibliotheek ziet de
gewone bouw? Wat ziet een audit die de bibliotheek als eigen code behandelt? En wat ziet ASan
als de bibliotheek al gebouwd is?

## Achtergrond

CMake voegt de headers van een geïmporteerde bibliotheek in als `-isystem`. Dat geldt voor elke
bibliotheek uit `find_package`, pixi of conda. De compiler meldt dan niets over die headers.
Dat is de bedoeling: je wilt geen honderden meldingen over code die je niet beheert. Maar het
verbergt ook de echte fouten.

Een bibliotheek uit pixi of conda is al gebouwd, zonder sanitizer. ASan controleert alleen
code die met `-fsanitize=address` is gecompileerd. Een `inline`-functie uit een header wordt
in je eigen code gecompileerd. Een functie in een `.cpp` van de bibliotheek niet.

De bibliotheek `vreemd/minilib` speelt de code van anderen. De fouten V1–V4 staan in
[vreemd/README.md](../../vreemd/README.md).

## Verwachting

1. De gewone presets geven geen enkele waarschuwing over `minilib`.
2. De audit-preset meldt V1 (niet geïnitialiseerd) en V3 (versmalling). `-Wunsafe-buffer-usage`
   meldt de pointer-indexen bij V2 en V4, maar niet specifiek dat er een fout is.
3. ASan vangt V2 en V4 als `minilib` uit de bron is gebouwd.
4. ASan mist V4 als `minilib` al gebouwd is zonder ASan. V2 vangt ASan dan wel, omdat `ml_sum`
   `inline` is.
5. Geen sanitizer vangt V3: een conversie van `long` naar `unsigned` is geen ongedefinieerd
   gedrag.

## Opzet

| Bestand | Inhoud |
| --- | --- |
| `vreemd/minilib/` | De bibliotheek van anderen, met V1–V4 |
| `wrapper.hpp/.cpp` | Een veilige grens om `minilib`: `std::span` in plaats van pointer plus lengte, controle met `std::in_range`, `std::optional` bij een fout |
| `wrapper_test.cpp` | Unit tests van de wrapper |
| `demo_unsafe.cpp` | Roept `minilib` direct aan, zonder wrapper |

| Doel | Bibliotheek |
| --- | --- |
| `e003_demo_unsafe` | `minilib`: uit de bron, met de sanitizer van de preset |
| `e003_demo_unsafe_prebuilt` | `minilib_prebuilt`: altijd zonder sanitizer, zoals een binair pakket |

| Variant | Aanroep | Fout |
| --- | --- | --- |
| `sum` | `ml_sum(v.data(), 3)` | V1 en V2 |
| `count` | `ml_count(-1)` | V3 |
| `get` | `ml_get(v.data(), 3)` | V4 |

## Uitvoeren

```bash
pixi run check
```

De audit:

```bash
pixi run audit
```

## Resultaat

Gemeten op 2026-11-22 met GCC 15.3.0, Clang 23.1.6, clang-tidy 23.1.6 en cppcheck 2.22.

### Tijdens het bouwen

| Vangnet | V1 niet geïnitialiseerd | V2 één te ver | V3 versmalling | V4 geen grenscontrole |
| --- | --- | --- | --- | --- |
| Gewone presets, GCC en Clang (`-isystem`) | niets | niets | niets | niets |
| `clang-audit`, compiler | `-Wuninitialized` | `-Wunsafe-buffer-usage` (algemeen) | `-Wshorten-64-to-32` | `-Wunsafe-buffer-usage` (algemeen) |
| `clang-audit`, clang-tidy | `cppcoreguidelines-init-variables` | niets | niets | niets |
| cppcheck (in `pixi run audit`) | `uninitvar`, `legacyUninitvar` | niets | niets | niets |

Met `-isystem` gaf de bouw van `minilib`, de wrapper en de demo **nul** waarschuwingen. De
veilige code bouwt met `-Werror`, en toch slaagde de bouw.

`-Wunsafe-buffer-usage` meldt elke index via een rauwe pointer. De melding zegt niet dat er een
fout is. De melding zegt: hier kan het misgaan, en geen hulpmiddel kan het nagaan. In onze eigen
veilige code (e001–e003) gaf `-Wunsafe-buffer-usage` geen melding.

cppcheck houdt geen rekening met `-isystem`. In de gewone taak `pixi run cppcheck` vond
cppcheck V1 via de wrapper, die `minilib.hpp` invoegt. Die taak slaat `vreemd/` daarom over.
`pixi run audit` controleert `vreemd/` apart.

### Tijdens het uitvoeren

| Preset | `sum` (V1, V2) | `count` (V3) | `get` (V4) |
| --- | --- | --- | --- |
| `gcc-debug`, uit de bron | `32772`: willekeurig | `4294967295` | `0`: willekeurig |
| `gcc-asan` en `clang-asan`, uit de bron | `heap-buffer-overflow` in `minilib.hpp:12` | `4294967295`, geen melding | `heap-buffer-overflow` in `minilib.cpp:6` |
| `gcc-asan` en `clang-asan`, **al gebouwd** | `heap-buffer-overflow` in `minilib.hpp:12` | `4294967295`, geen melding | **`0`, geen melding** |
| Clang `-fsanitize=implicit-conversion` | niet gemeten | `implicit conversion ... changed the value to 4294967295` | niet gemeten |

`-fsanitize=implicit-conversion` is apart gemeten, buiten de presets.

## Conclusie

1. Verwachting 1 klopt. De gewone bouw is stil over code van anderen, ook met `-Werror` en
   alle waarschuwingen aan.
2. Verwachting 2 klopt. De audit vindt V1 en V3 precies. V2 en V4 vindt de audit alleen als
   "hier wordt een rauwe pointer geïndexeerd". Dat is een plek om te lezen, geen bewijs van een
   fout.
3. Verwachting 3 klopt. Met de bibliotheek uit de bron vangt ASan V2 en V4.
4. Verwachting 4 klopt. Met de al gebouwde bibliotheek ziet ASan V4 niet. V2 ziet ASan wel,
   omdat `ml_sum` `inline` is en in onze eigen, geïnstrumenteerde code wordt gecompileerd.
5. Verwachting 5 klopt. V3 is gedefinieerd gedrag. ASan en UBSan zwijgen. Alleen een
   waarschuwing tijdens het bouwen vindt V3 vroeg. Die waarschuwing verbergt `-isystem`.
   Clang heeft ook `-fsanitize=implicit-conversion`, die V3 tijdens het uitvoeren vangt.
6. Geen hulpmiddel vond de off-by-one van V2 als zodanig. Alleen ASan vond de gevolgen, en
   alleen omdat de test de functie aanroept.

## Herkennen in een code-review

| Signaal | Waarom verdacht |
| --- | --- |
| Een functie die `T*` en een lengte krijgt | Wie controleert dat de lengte klopt bij de pointer? |
| `<=` in een lus over een array | Klassieke off-by-one |
| Een lokale variabele zonder beginwaarde | Lezen voor schrijven is ongedefinieerd gedrag |
| Een `return` van een breder type naar een smaller type zonder cast | Stille versmalling |
| "De aanroeper moet ervoor zorgen dat ..." in het commentaar | Een voorwaarde die niemand controleert |

## Veilig alternatief

| Situatie | Alternatief |
| --- | --- |
| Een nieuwe bibliotheek van anderen | Draai eerst `pixi run audit`. Lees elke melding. |
| Een bibliotheek als binair pakket | Bouw de bibliotheek uit de bron in de presets met een sanitizer. Lukt dat niet, noteer de bibliotheek als blinde vlek. |
| Aanroepen naar de bibliotheek | Eén wrapper aan de grens, met `std::span`, `std::in_range` en `std::optional` of `std::expected`. Test de wrapper onder ASan. |
| Een functie van anderen die aantoonbaar fout is | Laat de wrapper de functie niet gebruiken (zoals `e003::sum` met `ml_sum` doet) |

## Bronnen

- CMake: [`SYSTEM`](https://cmake.org/cmake/help/latest/prop_tgt/SYSTEM.html) en
  `INTERFACE_SYSTEM_INCLUDE_DIRECTORIES` bij geïmporteerde doelen.
- Clang: [C++ Safe Buffers](https://clang.llvm.org/docs/SafeBuffers.html) (`-Wunsafe-buffer-usage`).
- Clang: [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html),
  groep `implicit-conversion`.
- cppreference: [`std::in_range`](https://en.cppreference.com/w/cpp/utility/in_range).
