# Summary
Submitted to UCLA's Student Research Program under the guidance of Professor Adnan Darwiche, this project uses Probabilistic Sentential Decision Diagram (PSDD) to encode Context-Free Grammar (CFG) and compares its effectiveness in inducing a probability distribution over structured space against Probabilistic Context-Free Grammar (PCFG). The experimental results suggest that PSDD can outperform PCFG in parsing ambiguous sentence structures.

# Sentential Decision Diagrams
Sentential Decision Diagram (SDD) is a new representation of propositional knowledge base that has applications in areas such as diagnosis, verification, system design, and planning. It results from imposing structured decomposability and strong deterministic decompositions as two properties on Negation Normal Form (NNF). Probabilistic Sentential Decision Diagram (PSDD) expands upon SDD to develop a representation of probability distributions in the presence of logical constraints. Some of the applications of this new representation include configuration problems that deal with choosing certain options for a product under given constraints. Others include verification and diagnosis, where propositional logic is used to encode system functionalities and learn probabilistic models subject to given constraints.

# Experiment & Results
The project encoded the following Context-Free Grammar:

S → AT
T → AU
T → BU
U → AV
U → AZ
V → AA
V → BA
Z → AA
A → a
B → b

where {S} is the start symbol, {A, T, U, V, Z} are non-terminals, and {a, b} are terminals. All possible sentences in the language are shown in derivations below:

S → AT → aAU → aaAV → aaaAA → aaaaa
S → AT → aAU → aaAV → aaaBA → aaaba
S → AT → aAU → aaAZ → aaaAA → aaaaa
S → AT → aBU → abAV → abaAA → abaaa
S → AT → aBU → abAV → abaBA → ababa
S → AT → aBU → abAZ → abaAA → abaaa

There are 4 unique sentences of length 5, namely {aaaaa, aaaba, abaaa, ababa}. While there’s only one way to parse {aaaba} and {ababa}, there are two ways to parse {aaaaa} and {abaaa}, making the latter two ambiguous. We simulated our own dataset with the following weights to compare PSDD's and PCFG's performance:

(1.0) S → AT
(0.9) T → AU
(0.1) T → BU
(0.8) U → AV
(0.2) U → AZ
(0.7) V → AA
(0.3) V → BA
(1) Z → AA
(1.0) A → a
(1.0) B → b

When we generated a PSDD from the existing SDD by training over the simulated dataset, we observed a negative log likelihood of ll1 = 0.876. When we generated a PCFG by learning weights using the inside-outside algorithm, we observed a negative log likelihood of ll2 = 2.33. Minimizing the negative log likelihood is equivalent to maximizing the likelihood function, and since 0.876 = ll1 < ll2 = 2.33, we conclude that the PSDD better represents CFG than PCFG by a measure of log likelihood.
