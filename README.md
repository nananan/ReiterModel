### Introduzione 

Gli  Automi Cellulari (AC) sono modelli di calcolo parallelo la cui  evoluzione è regolata da leggi puramente locali. Nella sua definizione  essenziale, un AC può essere descritto come uno spazio suddiviso in  celle regolari, ognuna delle quali può trovarsi in un numero finito di  stati. Ogni cella dell'AC ingloba, infatti, un automa finito, che è uno  dei modelli di calcolo più semplici.  

Al  tempo t=0 le celle sono in uno stato arbitrario e l'AC evolve cambiando  gli stati delle celle a passi discreti di tempo applicando  simultaneamente a ognuna la stessa legge, o funzione, di transizione. 

L'input  per ciascuna cella è dato dagli stati delle celle vicine e le  condizioni di vicinato sono determinate da una relazione geometrica,  invariante nel tempo e nello spazio. 

Nello  specifico, gli AC si prestano particolarmente bene alla modellizzazione  e simulazione di quei sistemi caratterizzati da numerosi costituenti  elementari in mutua interazione. 

Un automa cellulare è caratterizzato dalle seguenti proprietà fondamentali: 

- è formato da uno spazio d-dimensionale suddiviso in celle regolari o, equivalentemente, da un reticolo regolare d-dimensionale; 
- il numero di stati della cella è finito 
- l'evoluzione avviene a passi discreti 
- ogni cella cambia stato simultaneamente a tutte le altre in accordo alla stessa regola di transizione 
- la regola di transizione dipende dallo stato della cella stessa e dallo stato delle celle vicine 
- la relazione di vicinanza è locale, uniforme e invariante nel tempo 

La vicinanza di un automa cellulare può essere, ad esempio: 

![1527968377496](./ImageRelation/1527968377496.png)

<div class="pagebreak"></div>

### Modello di Reiter 

Il modello di Reiter è un automa esagonale. Data una tassellazione del piano in celle esagonali, ogni cella $z$ ha sei vicini. Si denota $s_t(z)$ lo stato variabile della cella $z$ al tempo $t$ che fornisce la quantità di acqua immagazzinata in $z$. 

Le celle sono divise in 3 tipi: 

- Una cella $z$ è "frozen" se $s_t(z) \ge 1$. 

- Se una cella non è "frozen" ma almeno un vicino è "frozen", la cella sarà una cella "boundary". 
- Una cella che non è né "frozen" e nemmeno "boundary" è chiamata "nonreceptive". 
- L'unione delle celle "frozen" e di quelle "boundary" sono chiamate celle "receptive" 

![1527968377496](./ImageRelation/reiter.png)

La condizione iniziale nel modello di Reiter è: 

![1527968377496](./ImageRelation/initialReiter.png)

In cui $\mathcal{O}$ è la cella di origine e $\beta$ rappresenta una costante fissata del livello di vapore di background. 

Inoltre si deifinisce 

- $u_t(z)$, la quantità di acqua che partecipa nella diffusione 
- $v_t(z)$, la quantità di acqua che non partecipa nella diffusione 

Quindi: 
$$
s_t(z) =  u_t(z) + v_t(z)
$$
E si fissa $v_t(z) = s_t(z)$ se $z$ è "receptive", e $u_t(z) = 0$ se $z$ è "non-receptive". 

Per $\gamma$, $\alpha$ due costanti fissate rappresentanti, rispettivamente, il vapore aggiunto e il coefficiente di diffusione, nel modello di Reiter lo stato della cella si  evolve in funzione degli stati dei suoi vicini più prossimi secondo due  regole di aggiornamento locale che riflettono i modelli matematici  sottostanti: 

- *Costante aggiuntiva*. Per ogni cella "receptive" $z$. 

  $$
  v_t^+(z) = v_t^-(z) + \gamma
  $$

   *Diffusione*. Per ogni cella z: 						 
  $$
  u_t^+(z) = u_t^-(z) + \frac{\alpha}{2} (\overline{u}_t^-(z) - u_t^-(z))
  $$




in cui si usa $\pm$ per denotare lo stato della cella prima e dopo che lo step sia completo, mentre $\overline{u}_t^-(z)$ definisce la media di $u_t^-$ per i sei vicini più prossimi della cella $z$.

Le celle ai lati sono considerate come "edge cells", in cui il valore è $u_t^+(z) = \beta$. Così, l'acqua è aggiunta al sistema attraverso queste celle nel processo di diffusione.

Conbinando le due variabili, si ottiene: 
$$
s_{t+1}(z) = u_t^+(z) + v_t^+(z)
$$
