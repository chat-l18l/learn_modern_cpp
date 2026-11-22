# e005 — Sterke types: handles, grootheden en verwisselde parameters

**Catalogus:** G28, G29 · **Principe:** P2 regels 6, 8 en 10 · **Besluit:** B20 · **Status:** onderzocht · **Datum:** 2026-11-22

## Vraag

1. Welke fouten met handles en grootheden houdt de compiler tegen als het type sterk is? Kan dat
   als test in de repo staan, zodat een latere wijziging aan het type de bouw breekt?
2. G28: een tabel waaruit je verwijdert, hergebruikt een slot. Een oude index leest dan het
   verkeerde element. Ziet een vangnet dat? Lost een handle met generatie het op, en waar houdt
   die oplossing op?
3. G29: twee parameters van hetzelfde type worden verwisseld. Helpt een sterk type? Zo niet, wat
   dan wel?

## Achtergrond

Een kale `std::uint16_t` als index past in elke tabel en in elke rekensom. Een sterk type
(`struct Sensor_id` met een `explicit` constructor) laat de compiler een verkeerd gebruik
weigeren. Zie P2 regel 6 in [PRINCIPES.md](../../docs/PRINCIPES.md).

Een verouderde index is geen ongedefinieerd gedrag: het slot bestaat en bevat een geldig
element, alleen het verkeerde. Een handle met generatie onthoudt hoe vaak een slot is
hergebruikt, en weigert een handle met een oude generatie.

## Verwachting

1. De `static_assert`-tests compileren met GCC en Clang. Een handle van een andere tabel, een
   `int`, `{3}`, rekenen en ordenen worden geweigerd. Een `Deci_celsius` past niet waar een
   `Millivolt` hoort.
2. G28: ASan meldt niets bij een verouderde gewone index. De pool met generatie weigert de oude
   handle.
3. Een generatie van 16 bits loopt na 65536 keer hergebruik rond. Daarna is een heel oude handle
   weer geldig.
4. G29: een sterk type helpt niet als beide parameters hetzelfde type hebben:
   `configure(Millivolt high, Millivolt low)` compileert. Een struct met benoemde velden en
   designated initializers helpt wel: `Limits{.high = ..., .low = ...}` in de verkeerde
   volgorde compileert niet.
5. clang-tidy `bugprone-easily-swappable-parameters` meldt `configure(int, int)` en ook
   `configure(Millivolt, Millivolt)`.

## Opzet

| Bestand | Inhoud |
| --- | --- |
| `units.hpp/.cpp` | `Millivolt`, `Deci_celsius`, en `Limits` met benoemde velden; `limits_check`, `limits_contain` |
| `sensor_table.hpp/.cpp` | Vaste tabel zonder verwijderen: `Sensor_id`, `sensor_add`, `sensor_at` (assert), `sensor_find` (een getal van buiten wordt een handle of een fout) |
| `sensor_pool.hpp/.cpp` | Pool met verwijderen: `Sensor_handle` met index en generatie, `sensor_pool_add`, `_remove`, `_at`, `_find` |
| `strong_types_test.cpp` | Deel 1: `static_assert`-tests tijdens het compileren. Deel 2: unit tests. |
| `compile_fail/*.cpp` | Vier bestanden die **niet** mogen compileren |
| `demo_bugs.cpp` | Drie fouten zonder ongedefinieerd gedrag: `stale-index`, `swapped-raw`, `swapped-strong` |

| Soort test | Wat de test bewijst |
| --- | --- |
| `static_assert` met een `concept` | Een eigenschap van een type. Breekt iemand het type, dan compileert de test niet meer. |
| Compile-fail-test (`leren_compile_fail_test`) | Een bestand compileert niet, en de compiler noemt de verwachte reden. Voor wat een `concept` niet kan uitdrukken, zoals de volgorde van designated initializers. |
| Blinde-vlek-test | De demo geeft de foute uitvoer, en geen sanitizer meldt iets |

## Uitvoeren

```bash
pixi run check
```

Alleen de compile-fail-tests:

```bash
ctest --test-dir build/gcc-debug -L compile-fail --output-on-failure
```

## Resultaat

Gemeten op 2026-11-22 met GCC 15.3.0, Clang 23.1.6, clang-tidy 23.1.6 en cppcheck 2.22.

### Tijdens het compileren

Alle 24 `static_assert`s in `strong_types_test.cpp` gelden met GCC en Clang. Een greep:

| Fout | Geweigerd |
| --- | --- |
| `Actuator_id` of `int` waar een `Sensor_id` hoort | ja |
| `sensor_at(table, {3})` | ja, door de `explicit` constructor |
| `id + 1`, `id < id`, `Sensor_handle == Sensor_id` | ja |
| `Deci_celsius` of `int` waar een `Millivolt` hoort | ja |
| Een `Millivolt` zonder waarde (`Millivolt` heeft geen default constructor) | ja |

Bewezen dat de tests kunnen falen: zonder `explicit` op `Sensor_id` stoppen vier
`static_assert`s de bouw. Een compile-fail-bestand dat wél compileert, laat zijn test falen.

### Compile-fail-tests

| Bestand | GCC 15 | Clang 23 |
| --- | --- | --- |
| `handle_of_other_table.cpp` | `could not convert 'e005::Actuator_id(0)' ... to 'e005::Sensor_id'` | `no viable conversion from 'e005::Actuator_id' to 'Sensor_id'` |
| `celsius_as_millivolt.cpp` | `could not convert 'e005::Deci_celsius(215)' ... to 'e005::Millivolt'` | `no viable conversion from 'e005::Deci_celsius' to 'Millivolt'` |
| `missing_field.cpp` | fout: `Millivolt` heeft geen default constructor | fout: idem |
| `designators_out_of_order.cpp` | fout | **alleen een waarschuwing** (`-Wreorder-init-list`); een fout door `-Werror` |

Designated initializers in de verkeerde volgorde zijn volgens de standaard een fout. GCC meldt
`designator order for field ... does not match declaration order`. Clang accepteert de code als
extensie, met een waarschuwing. Zonder `-Werror` compileert de verwisselde volgorde dus met Clang.

### G28 en G29 tijdens het uitvoeren

| Demo | Uitvoer, elke preset | Sanitizer |
| --- | --- | --- |
| `stale-index` | `read via old index: channel 2` (verwacht: 1) | niets |
| `swapped-raw` | `alarm for 500 mV: yes` (verwacht: no) | niets |
| `swapped-strong` | `alarm for 500 mV: yes` (verwacht: no) | niets |

| Pool met generatie | Uitkomst |
| --- | --- |
| Oude handle na hergebruik van het slot | `Pool_error::stale_handle` |
| Twee keer verwijderen | `Pool_error::stale_handle` |
| Na 65536 keer verwijderen en toevoegen op één slot | De eerste handle is **weer geldig** en wijst de nieuwe sensor aan |

### Statische analyse

| Hulpmiddel | Melding | Waar |
| --- | --- | --- |
| clang-tidy `readability-suspicious-call-argument` | "1st argument 'high_mv' (passed to 'low_mv') looks like it might be swapped" | `swapped-raw` en `swapped-strong` |
| clang-tidy `bugprone-easily-swappable-parameters` | "2 adjacent parameters ... of similar type" | de constructor `Sensor_handle(index, generation)` in onze eigen veilige code |
| clang-tidy `bugprone-easily-swappable-parameters` | niets | `alarm_raw(int, int, int)` en `alarm_strong(Millivolt, Millivolt, Millivolt)` in de demo |
| cppcheck `knownConditionTrueFalse` | "Condition 'alarm' is always true" | `swapped-raw`: cppcheck rekent het gevolg van de verwisseling uit |
| clang-tidy `performance-enum-size` | de error-enums kunnen `std::uint8_t` zijn | overgenomen |

## Conclusie

1. Verwachting 1 klopt. Een sterk type maakt fouten tot compileerfouten, en `static_assert` met
   een `concept` legt dat vast als test. Een `requires`-expressie buiten een template werkt
   daarvoor niet.
2. Verwachting 2 klopt. De verouderde index leest een geldig, verkeerd element. ASan en UBSan
   zwijgen. De pool met generatie weigert de oude handle.
3. Verwachting 3 klopt. Na 65536 keer hergebruik is een oude handle weer geldig. Een generatie
   verkleint de kans op G28, maar sluit G28 niet uit. Een vaste tabel zonder verwijderen (P2
   regel 6) sluit G28 wel uit.
4. Verwachting 4 klopt. Een sterk type ziet geen verwisseling van twee waarden van hetzelfde type.
   Een struct met benoemde velden wel, maar met Clang alleen dankzij `-Werror`.
5. Verwachting 5 klopt niet. `bugprone-easily-swappable-parameters` meldde de demo niet, maar
   wel de constructor van `Sensor_handle`. De verwisselingen zelf vond
   `readability-suspicious-call-argument`, omdat de namen van de argumenten op de namen van de
   parameters lijken. Of de controle iets vindt als de namen niet op elkaar lijken, is niet
   gemeten.
6. cppcheck vond het gevolg van een verwisseling: een voorwaarde die altijd waar is.

## Herkennen in een code-review

| Signaal | Waarom verdacht |
| --- | --- |
| Een index als `std::size_t`, `int` of `std::uint16_t` in een interface | Past in elke tabel en in elke som |
| Een tabel waaruit wordt verwijderd, met gewone indexen | Een oude index leest na hergebruik het verkeerde element |
| Twee of meer naast elkaar staande parameters van hetzelfde type | Een verwisseling compileert, ook met sterke types |
| Een fysieke waarde als `int` of `float` zonder eenheid in de naam | Een andere eenheid past er ook in |
| Een struct met een constructor met twee velden van hetzelfde type | Zoals `Sensor_handle(index, generation)`: G29 in een handle |

## Veilig alternatief

| Situatie | Alternatief |
| --- | --- |
| Een element aanwijzen | Een sterk handle-type per tabel (P2 regel 6) |
| Een tabel zonder verwijderen | Een gewone handle volstaat; G28 kan niet optreden |
| Een tabel met verwijderen | Een handle met generatie (P2 regel 8). Bij een kleine generatie: een slot dat zou rondlopen, niet meer gebruiken. |
| Een fysieke waarde | Een sterk type met de eenheid in de naam (P2 regel 10) |
| Twee waarden van hetzelfde type | Een struct met benoemde velden en designated initializers, met `-Werror` |

## Bronnen

- cppreference: [aggregate initialization](https://en.cppreference.com/w/cpp/language/aggregate_initialization),
  onderdeel "Designated initializers".
- cppreference: [constraints and concepts](https://en.cppreference.com/w/cpp/language/constraints),
  onderdeel "Requires expressions".
- clang-tidy: [`readability-suspicious-call-argument`](https://clang.llvm.org/extra/clang-tidy/checks/readability/suspicious-call-argument.html),
  [`bugprone-easily-swappable-parameters`](https://clang.llvm.org/extra/clang-tidy/checks/bugprone/easily-swappable-parameters.html).
