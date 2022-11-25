# taas-harper++
Matthias Thimm (matthias.thimm@fernuni-hagen.de)

taas-harper++ is written in C and in order to compile it you
need glib 2.0 header files (https://developer.gnome.org/glib/).

Then compile taas-harper++ via
```
    ./build-taas-harper++.sh
```
taas-harper++ uses a ICCMA23-compliant command line interface, see also
https://iccma2023.github.io/rules.html.

taas-harper++ implements the problems [DC-CO,DS-CO,DC-PR,DS-PR,DC-ST,DS-ST,
DC-SST,DS-SST,DC-STG,DS-STG,DS-ID] and supports the ICCMA23 format for abstract
argumentation frameworks. In particular, to decide skeptical acceptance
of an argument wrt. preferred semantics of an AAF in TGF format use
```
  taas-harper++ -p DS-PR -fo i23 -f <file in ICCMA23 format> -a <argument>
```
