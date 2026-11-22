# Ontwerpprincipes

Deze principes beschrijven betrouwbare, eenvoudige software: minimale abstracties, geen diepe
klassenhiërarchieën, en data en control flow die je kunt volgen. Elk principe heeft een regel,
een reden, een toets en bewijs.

De principes gelden voor alle nieuwe code in dit project. Ze zijn ook bedoeld om code en
architectuur van andere projecten te toetsen, en code van anderen. De vragen voor een audit staan
in [AUDITVRAGEN.md](AUDITVRAGEN.md).

## Overzicht

| ID | Principe | Kern |
| --- | --- | --- |
| P1 | Data en control flow zijn gescheiden | Data in structs, transformaties in functies, beslissingen op één plek |
| P2 | Elke data heeft één eigenaar | Eén plek beheert het geheugen en schrijft. De rest leent, korter dan de eigenaar leeft. |
| P3 | Control flow is expliciet | Toestanden in een `enum class`, overgangen op één plek, geen verborgen sprongen |
| P4 | Minimale abstractie | Concrete code. Elke abstractie heeft een reden die je kunt aanwijzen. |
| P5 | Geheugengebruik is vast en zichtbaar | De aanroeper alloceert. Een stackbudget per functie. |
| P6 | Code van anderen staat achter één grens | Eerst controleren, dan gebruiken, en alleen via een wrapper |

| Status | Betekenis |
| --- | --- |
| vastgesteld | Het principe heeft een toets en minstens één verwijzing naar bewijs |
| werkhypothese | Het principe heeft nog geen bewijs uit een experiment |

| ID | Status | Bewijs |
| --- | --- | --- |
| P1 | vastgesteld | e004 |
| P2 | vastgesteld | e001, e002, e004 |
| P3 | werkhypothese | G27: voorlopige meting, nog geen experiment |
| P4 | werkhypothese | – |
| P5 | vastgesteld | e004, B16 |
| P6 | vastgesteld | e003 |

---

## P1 — Data en control flow zijn gescheiden

### Regel

1. Data staat in eenvoudige structs, zonder gedrag.
2. Een transformatie is een vrije functie: data in, data uit. Een transformatie is bij voorkeur
   een pure functie.
3. De control flow van een taak (wat er wanneer gebeurt) staat op één plek: een state machine of
   een tabel. De control flow zit niet verspreid over callbacks, virtuele functies of namen van
   functies.
4. Invoer en uitvoer (hardware, bestanden, netwerk, tijd) staan in weinig functies met een
   duidelijke naam. Het rekenwerk staat ertussen, in pure functies.

### Waarom

Een fout zoeken is twee vragen beantwoorden. Welke data was fout? Welke beslissing leidde
daartoe? Als data en control flow door elkaar lopen, moet je bij elke fout reconstrueren hoe het
geheel is samengesteld. Als ze gescheiden zijn, beantwoord je de twee vragen apart. Een audit en
een uitleg worden daardoor korter.

Een pure functie is eenvoudig te testen. Dezelfde invoer geeft altijd dezelfde uitkomst, en de
test hoeft geen toestand op te bouwen.

### Toets

| Toets | Soort |
| --- | --- |
| A1–A5, A9 | auditvraag |
| Een variabele of parameter die `const` kan zijn | clang-tidy `misc-const-correctness` |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e004](../experimenten/e004-lambda-capture/README.md) | Een callback met `[&count]` verbergt data (een pointer), eigenaar en levensduur in de control flow. Uitgeschreven als struct `Counter`, functie `counter_tick` en tabel `Counter_table` zijn alle drie zichtbaar, en kan niets meer hangen. |

### Achtergrond

| Bron | Gedachte |
| --- | --- |
| Fred Brooks, *The Mythical Man-Month* (1975) | "Show me your tables, and I won't usually need your flowcharts." |
| Rob Pike, *Notes on Programming in C* (1989), regel 5 | "Data dominates." |
| Gary Bernhardt, *Boundaries* (2012) | "Functional core, imperative shell": pure kern, dunne schil met invoer en uitvoer |
| Mike Acton, *Data-Oriented Design and C++* (CppCon 2014) | Eerst de data, dan de code |
| Melvin Conway, *How Do Committees Invent?* (1968) | Een systeem neemt de structuur over van de communicatie in de organisatie die het bouwt |

---

## P2 — Elke data heeft één eigenaar

### Regel

1. Elk object met data heeft één eigenaar. De eigenaar beheert het geheugen en is de enige die
   de data mag veranderen.
2. Een functie die de data alleen leest, krijgt `const T &` of `std::span<const T>`.
3. Een functie die de data mag veranderen, krijgt `T &` of `std::span<T>`, voor de duur van
   één aanroep.
4. Een view, referentie, pointer, iterator of referentie-capture leeft niet langer dan de
   aanroep. Een uitzondering heeft commentaar dat de eigenaar en de levensduur noemt.
5. De invarianten van de data staan bij de data. Twee manieren, bij voorkeur de eerste:
   - types maken een ongeldige toestand onmogelijk: `enum class`, `std::variant`,
     `std::array` met een vaste grootte;
   - de transformatiefunctie controleert de invariant: `assert` bij een programmeerfout,
     `std::expected` bij invoer van buiten.

Regel 5 is nodig omdat P1 de data losmaakt van het gedrag. Object-oriëntatie beschermt
invarianten door data in een klasse te verbergen. Zonder die klasse moet de bescherming uit de
types en uit de transformaties komen.

### Waarom

Alle constructies in fase 1 van de [catalogus](CATALOGUS.md) zijn fouten in eigenaarschap:
iets leent van een eigenaar die al weg is. Een fout in eigenaarschap is moeilijk te testen. Het
vrijgegeven geheugen bevat vaak nog de oude waarde, en de test is groen.

### Toets

| Toets | Soort |
| --- | --- |
| A1–A5 | auditvraag |
| `[[clang::lifetimebound]]` op een parameter waarvan het resultaat leent | Clang |
| `-Wlifetime-safety` | Clang 23 |
| `invalidContainer`, `returnDanglingLifetime`, `invalidLifetime` | cppcheck |
| `heap-use-after-free`, `stack-use-after-scope`, `stack-use-after-return` | ASan |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e001](../experimenten/e001-dangling-string-view/README.md) | Een `std::string_view` naar een tijdelijke `std::string`. Alleen ASan vindt het altijd. Zonder sanitizer geeft een korte string het juiste antwoord. |
| [e002](../experimenten/e002-vector-push-back/README.md) | Een referentie in een vector na `push_back`. Of de code fout is, hangt af van de capaciteit: een getal dat niet in de code staat. |
| [e004](../experimenten/e004-lambda-capture/README.md) | Een lambda die ontsnapt. In `gcc-debug` geeft de fout het juiste antwoord. |

---

## P3 — Control flow is expliciet

### Regel

1. De toestanden van een taak staan in één `enum class`.
2. Eén functie berekent de volgende toestand, met een `switch` over de huidige toestand zonder
   `default`. Er is één punt waar de nieuwe toestand wordt vastgelegd.
3. Een keuze tussen gevallen is een tabel, geen keten van `if`/`else`.
4. Geen control flow die je niet ziet op de plek van de aanroep:
   - geen overloads die alleen in parametertype verschillen;
   - geen impliciete conversies naar `bool` of tussen getaltypes;
   - geen virtuele aanroep of callback als een `switch` hetzelfde doet.

### Waarom

Een tabel of een `switch` laat in één oogopslag zien of elk geval is afgedekt. Een keten van
`if`'s doet dat niet. Bij een fout zie je de toestand en de gebeurtenis, en de tabel zegt wat er
had moeten gebeuren.

### Toets

| Toets | Soort |
| --- | --- |
| A6–A8 | auditvraag |
| `-Wswitch` (in `-Wall`), `-Wimplicit-fallthrough` | compiler |
| `-Wconversion`, `-Wsign-conversion`, `-Wstring-conversion` | compiler |
| `readability-implicit-bool-conversion` | clang-tidy |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [G27](CATALOGUS.md) (voorlopige meting) | `log_it("hello")` roept `log_it(bool)` aan, niet `log_it(const std::string &)`. GCC 15 meldt niets. |

---

## P4 — Minimale abstractie

### Regel

| Regel | Uitzondering |
| --- | --- |
| Schrijf concrete functies en structs. Een template alleen als dezelfde logica voor minstens drie echt verschillende types nodig is. | – |
| Begrens elke template-parameter met een C++20 `concept` of een `requires`-clausule. | – |
| Geef elke functie een eigen naam: `parse_from_file`, `parse_from_string`. | Constructors, operators (`==`, `<=>`, `<<`), en namen die de standaardbibliotheek voorschrijft |
| Houd gedrag bij elkaar. Liever één duidelijke functie van 40 regels dan tien helpers verspreid over bestanden. Splits pas als het deel een eigen naam verdient of een eigen test nodig heeft. | – |
| `virtual` alleen voor runtime-polymorfie met een open verzameling types. Een gesloten verzameling is een `enum class` met een `switch`, of een `std::variant`. | – |
| Een klasse met `virtual` heeft een `virtual` of `protected` destructor. Geef een object van zo'n klasse nooit by value door. | – |
| Een lambda die ontsnapt (teruggegeven, in een `std::function`, als callback) vangt niets by reference en gebruikt geen `[=]` in een lidfunctie. | Een lambda die direct draait binnen de scope (`std::sort`, `std::ranges::for_each`) mag `[&]` gebruiken |
| Commentaar legt uit waarom, en wat het contract is: eigenaar, levensduur, voorwaarden. Geen commentaar dat herhaalt wat de code zegt. | In een demo mag commentaar het mechanisme uitleggen |

### Waarom

Elke abstractie verplaatst een deel van de betekenis naar een andere plek. Een lezer, een
auditor of een agent moet die plek dan vinden. Een abstractie is de moeite waard als zij meer
uitleg bespaart dan zij kost.

### Toets

| Toets | Soort |
| --- | --- |
| A10 | auditvraag |
| `readability-function-cognitive-complexity` | clang-tidy |
| `modernize-use-constraints` | clang-tidy |
| `-Wnon-virtual-dtor`; `cppcoreguidelines-virtual-class-destructor`, `cppcoreguidelines-slicing` | compiler, clang-tidy |
| `clang-analyzer-core.StackAddressEscape`, `cppcoreguidelines-misleading-capture-default-by-value` | clang-tidy |

### Bewijs

Nog geen experiment. Fase 6 van het [plan](PLAN.md) toetst dit principe aan code uit de
praktijk.

---

## P5 — Geheugengebruik is vast en zichtbaar

### Regel

Het ontwerpdoel is een stack van 4 KiB per taak. Op de stack staan parameters, referenties en
kleine lokale waarden. Grote gegevens staan er niet.

1. Een functie alloceert geen buffers. De aanroeper alloceert en geeft de buffer mee als
   `std::span<T>` (schrijven) of `std::span<const T>` (lezen).
2. Een functie geeft geen eigen container terug (`std::string`, `std::vector`). Het resultaat
   gaat in een buffer van de aanroeper. De functie geeft terug hoeveel zij schreef, of een fout
   via `std::expected`.
3. Grote gegevens staan in `static` opslag of in een struct die de eigenaar is.
4. Elke functie past in het stackbudget van 512 bytes (`LEREN_FRAME_LIMIT`).
5. Een struct met toestand krijgt een `static_assert` op `sizeof`.
6. Geen `std::function` in code voor een apparaat. Een `std::function` gebruikt de heap boven 16
   bytes aan captures, zonder dat de code dat laat zien.

| Uitzondering op het stackbudget | Waarom |
| --- | --- |
| Tests (`leren_gtest`, markering `HOST`) | Draaien alleen op de ontwikkelmachine |
| Demo's (`UNSAFE`) | Hun meldingen zijn meetgegevens |
| Presets met een sanitizer | ASan maakt stackframes groter. Een apparaat draait niet met ASan. |

### Waarom

Het geheugengebruik is dan vooraf bekend en staat in de code. Een stackoverflow op een apparaat
is een fout die je pas in het veld ziet. Een te groot stackframe dat niet compileert, zie je
aan je bureau.

### Toets

| Toets | Soort |
| --- | --- |
| A11–A13 | auditvraag |
| `-Wframe-larger-than=512` met `-Werror` | compiler, presets zonder sanitizer |
| `-fstack-usage` | compiler: stackframe per functie |
| `static_assert(sizeof(T) <= N)` | compiler |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e004](../experimenten/e004-lambda-capture/README.md) | `[buffer]` met 4 KiB verdubbelt het stackframe: 8208 tegen 4128 bytes. Een `static` buffer: 16 bytes. `std::function` alloceert boven 16 bytes aan captures. |
| [B16](BESLUITEN.md) | Een geplante buffer van 600 bytes stopt de bouw. De grootste functie in de bibliotheekcode gebruikt 128 bytes. |

De code in e001–e004 dateert van vóór deze regel. `e001::first_word_copy` geeft een
`std::string` terug en `e002::append_evens` laat de vector groeien. Die code blijft staan als
studiemateriaal.

---

## P6 — Code van anderen staat achter één grens

### Regel

1. Controleer code van anderen voordat zij in gebruik komt, en na elke update: `pixi run audit`.
2. Bouw code van anderen uit de bron mee in de presets met een sanitizer. Een bibliotheek die
   alleen als binair bestand bestaat, staat in het verslag als blinde vlek.
3. Roep code van anderen alleen aan via één wrapper. De wrapper gebruikt `std::span`,
   `std::in_range` en `std::optional` of `std::expected`. De wrapper controleert elke
   voorwaarde die de code van anderen aan de aanroeper overlaat.
4. Een functie van anderen die aantoonbaar fout is, gebruikt de wrapper niet.

### Waarom

De gewone bouw is stil over code van anderen. CMake voegt hun headers in als `-isystem`, en de
compiler meldt dan niets. ASan ziet niets in een bibliotheek die zonder ASan is gebouwd.

### Toets

| Toets | Soort |
| --- | --- |
| A14, A15 | auditvraag |
| `pixi run audit`: preset `clang-audit`, clang-tidy en cppcheck over `vreemd/` | hulpmiddel |
| ASan met de bibliotheek uit de bron | sanitizer |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e003](../experimenten/e003-code-van-anderen/README.md) | Vier geplante fouten: de gewone bouw meldt 0. De audit vindt er 2 precies en wijst de andere 2 aan. ASan mist de fout in de al gebouwde bibliotheek. |
