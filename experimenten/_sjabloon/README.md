# @ID@ — @TITEL@

**Catalogus:** Gxx · **Status:** bezig · **Datum:** @DATUM@

## Vraag

Welke vraag beantwoordt dit experiment? Eén of twee zinnen.

## Achtergrond

Wat zegt de standaard? Wat was er twintig jaar geleden anders? Een klein codevoorbeeld.

## Verwachting

1. Wat verwacht je van de compilers?
2. Wat verwacht je van clang-tidy en cppcheck?
3. Wat verwacht je van de sanitizers?
4. Wat doet het programma zonder sanitizer?

## Opzet

| Bestand | Inhoud |
| --- | --- |
| `example.hpp/.cpp` | De API die we bestuderen |
| `example_test.cpp` | Unit tests van de veilige vorm |
| `demo_unsafe.cpp` | De onveilige vorm |

## Uitvoeren

```bash
pixi run check
```

## Resultaat

Gemeten op @DATUM@ met (versies invullen).

| Vangnet | Variant 1 | Variant 2 |
| --- | --- | --- |
| GCC | | |
| Clang | | |
| clang-tidy | | |
| cppcheck | | |
| ASan/UBSan | | |
| Zonder sanitizer | | |

## Conclusie

Per verwachting: klopt of klopt niet, en wat er werkelijk gebeurde.

## Herkennen in een code-review

| Signaal | Waarom verdacht |
| --- | --- |
| | |

## Veilig alternatief

| Situatie | Alternatief |
| --- | --- |
| | |

## Bronnen

- 
