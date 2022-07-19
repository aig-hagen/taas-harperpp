# taas-harper++ v1.0 (2020-12-19)
Matthias Thimm (thimm@uni-koblenz.de)

taas-harper++ is written in C and in order to compile it you
need glib 2.0 header files (https://developer.gnome.org/glib/).

Then compile taas-harper++ via
```
    ./build-taas-harper++.sh
```
taas-harper++ uses a probo-compliant command line interface, see also
http://argumentationcompetition.org/2021/SolverRequirements.pdf.

taas-harper++ implements the problems [DC-CO,DS-CO,DC-PR,DS-PR,DC-ST,DS-ST,
DC-SST,DS-SST,DC-STG,DS-STG,DS-ID] and supports the TGF format for abstract
argumentation frameworks. In particular, to decide skeptical acceptance
of an argument wrt. preferred semantics of an AAF in TGF format use
```
  taas-harper++ -p DS-PR -fo TGF -f <file in TGF format> -a <argument>
```
