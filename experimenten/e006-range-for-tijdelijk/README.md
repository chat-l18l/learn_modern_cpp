# e006 — Range-for over een deel van een tijdelijk object

**Catalogus:** G04 · **Principe:** P1, P2 · **Status:** onderzocht · **Datum:** 2026-11-22

## Vraag

1. `for (int x : readings_of(make_sensor_list()))` loopt over een deel van een tijdelijk object.
   In C++20 is dat ongedefinieerd gedrag, in C++23 niet. Klopt dat met GCC 15 en Clang 23?
2. Welk vangnet ziet het verschil?
3. Welke vorm is in beide standaarden veilig, en welke vorm is in beide fout?

## Achtergrond

Een range-for is een afkorting. De compiler maakt van

```cpp
for (int x : readings_of(make_sensor_list())) { use(x); }
```

ongeveer dit:

```cpp
{
    auto &&range = readings_of(make_sensor_list());  // the temporary list dies at the ;
    for (auto it = range.begin(); it != range.end(); ++it) { int x = *it; use(x); }
}
```

`range` is een referentie naar een deel van de tijdelijke `Sensor_list`. In C++20 eindigt die
tijdelijke lijst aan het einde van de eerste regel. De lus leest daarna uit een object dat niet
meer bestaat.

C++23 verandert die regel (voorstel P2718R0): alle tijdelijke objecten in de expressie na de `:`
leven tot het einde van de lus. Dezelfde code is dan correct.

Twee vormen vallen buiten die regel:

| Vorm | C++20 | C++23 | Waarom |
| --- | --- | --- | --- |
| `for (int x : make_sensor_list().readings)` | correct | correct | Een referentie naar een **lid** van een tijdelijk object verlengt het leven van het hele object. Dat gold al vóór C++23. |
| `const auto &r = readings_of(make_sensor_list()); for (int x : r)` | fout | fout | P2718 geldt alleen binnen de range-for. Een referentie met een naam valt erbuiten. |

De tweede vorm is de valkuil. Wie een correcte C++23-lus "netjes" herschrijft met een variabele
met een naam, maakt de fout opnieuw.

## Verwachting

1. Onder ASan, in C++20: `via-function` en `optional-value` geven een melding
   (`stack-use-after-scope`). In C++23 geven ze geen melding.
2. `direct-member` is in beide standaarden correct.
3. `named-ref` is in beide standaarden fout, en ASan meldt het.
4. GCC meldt `named-ref` met `-Wdangling-reference`. Clang meldt `named-ref` omdat
   `readings_of` `[[clang::lifetimebound]]` heeft.
5. Zonder sanitizer drukken de foute varianten de juiste getallen af: het geheugen van een
   tijdelijk object op de stack is niet overschreven.
6. cppcheck meldt `named-ref`.

## Opzet

| Bestand | Inhoud |
| --- | --- |
| `sensor_list.hpp/.cpp` | `Sensor_list` met het publieke lid `readings`; `readings_of` (leent, met `[[clang::lifetimebound]]`); `make_optional_list`; `sum_readings` |
| `sensor_list_test.cpp` | Unit tests van de vormen die in C++20 én C++23 correct zijn |
| `demo_unsafe.cpp` | Vier varianten. Gebouwd als `e006_demo_unsafe_cpp20` en `e006_demo_unsafe_cpp23`. |

| Variant | Code |
| --- | --- |
| `direct-member` | `for (int x : make_sensor_list().readings)` |
| `via-function` | `for (int x : readings_of(make_sensor_list()))` |
| `optional-value` | `for (int x : make_optional_list(true).value().readings)` |
| `named-ref` | `const auto &r = readings_of(make_sensor_list()); for (int x : r)` |

| Test | Wat de test bewijst |
| --- | --- |
| `e006_correct_cpp20_direct-member`, `e006_correct_cpp23_*` | Correcte code: slaagt onder elke preset, en geen sanitizer meldt iets |
| `e006_asan_catches_cpp20_*`, `e006_asan_catches_cpp23_named-ref` | ASan vangt het ongedefinieerde gedrag |

## Uitvoeren

```bash
pixi run check
```

```bash
build/gcc-asan/experimenten/e006-range-for-tijdelijk/e006_demo_unsafe_cpp20 via-function
```

## Resultaat

Gemeten op 2026-11-22 met GCC 15.3.0, Clang 23.1.6, clang-tidy 23.1.6 en cppcheck 2.22.

### ASan

| Variant | C++20 | C++23 |
| --- | --- | --- |
| `direct-member` | niets: correct | niets: correct |
| `via-function` | `stack-use-after-scope` | niets: correct |
| `optional-value` | `stack-use-after-scope` | niets: correct |
| `named-ref` | `stack-use-after-scope` | `stack-use-after-scope` |

GCC en Clang geven dezelfde uitkomst.

### Zonder sanitizer

Alle 8 combinaties van variant en standaard drukken `sum 10` af, het juiste antwoord, in
`gcc-debug`, `gcc-release` en `clang-debug`: 24 runs. Ook de combinaties met ongedefinieerd
gedrag.

### Compilers

| Variant | GCC 15, C++20 | GCC 15, C++23 | Clang 23, C++20 | Clang 23, C++23 |
| --- | --- | --- | --- | --- |
| `direct-member` | niets | niets | niets | niets |
| `via-function` | `-Wdangling-reference` | niets | `-Wdangling`, `-Wlifetime-safety-use-after-scope` | niets |
| `optional-value` | **niets** | niets | `-Wdangling-gsl`, `-Wlifetime-safety-use-after-scope` | niets |
| `named-ref` | `-Wdangling-reference` | `-Wdangling-reference` | `-Wdangling`, `-Wlifetime-safety-use-after-scope` | `-Wdangling`, `-Wlifetime-safety-use-after-scope` |

Beide compilers kennen de regel van C++23. In C++23 geven ze op `via-function` en
`optional-value` geen vals alarm.

### Statische analyse

| Hulpmiddel | Melding |
| --- | --- |
| clang-tidy | Geen melding over levensduur. Wel `bugprone-unchecked-optional-access` op `.value()` in de demo, en op `list->readings` in de test na `ASSERT_TRUE(list.has_value())`. Het tweede is vals alarm: clang-tidy begrijpt niet dat `ASSERT_TRUE` de test stopt. |
| cppcheck 2.22 | Geen melding over levensduur. Alleen `useStlAlgorithm` op de lussen. |

## Conclusie

1. De vraag klopt. Met GCC 15 en Clang 23 is `via-function` in C++20 ongedefinieerd gedrag en in
   C++23 correct. Dat geldt ook voor `optional-value`.
2. Verwachting 1 tot en met 3 kloppen. ASan scheidt de twee standaarden precies.
3. Verwachting 4 klopt maar half. Clang meldt in C++20 alle drie de fouten, en in C++23 alleen
   `named-ref`. GCC meldt `via-function` en `named-ref`, maar **niet** `optional-value`.
4. Verwachting 5 klopt. Zonder sanitizer geeft elke foute variant het juiste antwoord. Een test
   zonder sanitizer ziet niets.
5. Verwachting 6 klopt niet. cppcheck meldt geen enkele variant. clang-tidy ook niet.
6. `direct-member` is in beide standaarden correct: een lid van een tijdelijk object verlengt het
   leven van het hele object. Een functie die een referentie teruggeeft, doet dat niet. Publieke
   leden in een eenvoudige struct (P1) zijn hier veiliger dan een functie die toegang geeft.
7. De belangrijkste les: `named-ref` is de "nette" herschrijving van een correcte C++23-lus, en is
   in beide standaarden fout. P2718 beschermt alleen de expressie na de `:`.

## Herkennen in een code-review

| Signaal | Waarom verdacht |
| --- | --- |
| Na de `:` van een range-for een functie die een referentie teruggeeft, met een tijdelijk object als argument | Correct in C++23, fout in C++20. Hangt af van de standaard van het project. |
| `.value()`, `operator[]`, `.front()` of `->` op een tijdelijk object in een range-for | Zelfde patroon. GCC meldt het niet bij `std::optional::value()`. |
| `const auto &x = f(g());` waarbij `f` een referentie teruggeeft | Fout in elke standaard |
| Code die met C++23 correct is en ook in een project met een ouder niveau terechtkomt | Dezelfde regel code wordt ongedefinieerd gedrag |

## Veilig alternatief

| Situatie | Alternatief |
| --- | --- |
| Lussen over een deel van een tijdelijk object | Geef de eigenaar eerst een naam: `const Sensor_list list = make_sensor_list(); for (x : readings_of(list))` |
| De waarde is klein | Kopieer de waarde binnen dezelfde expressie: `const Readings r = readings_of(make_sensor_list());` |
| Eigen data | Een publiek lid in een eenvoudige struct (P1) in plaats van een functie die een referentie teruggeeft |
| Een functie die leent | `[[clang::lifetimebound]]` op de parameter, zodat Clang een tijdelijk object meldt |

## Bronnen

- P2718R0: *Wording for P2644R1 Fix for Range-based for Loop*, opgenomen in C++23.
- cppreference: [range-based for loop](https://en.cppreference.com/w/cpp/language/range-for),
  onderdeel "Temporary range initializer".
- cppreference: [lifetime of a temporary](https://en.cppreference.com/w/cpp/language/reference_initialization#Lifetime_of_a_temporary).
