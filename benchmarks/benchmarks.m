(* ::Package:: *)

(* ::Section:: *)
(*Data import*)


datapath="/data/BuurlageWits/benchmarks/";
rawdata=Import[datapath<>"benchmarks.txt","Table"];
data=Partition[SplitBy[rawdata,Length],2];
labels=ToString/@Flatten[data[[All,1,All,{2,3,4,5}]],1];
cycles=Map[N[{#[[2]]*8,#[[3]]}]&,data[[All,2]],{2}];
speeds=Map[N[{#[[2]]*8,#[[2]]*8*600/#[[3]]}]&,data[[All,2]],{2}];


(* ::Section:: *)
(*Mean speeds*)


TableForm[{labels,Map[Mean,speeds[[All,All,2]]]}]


(* ::Section:: *)
(*Speed plot*)


ListPlot[speeds[[All]],PlotLegends->labels[[All]],AxesLabel->{"Bytes","MB/s"},PlotRange->All]


(* ::Section:: *)
(*Clockcycles plot*)


(* ::Subsection:: *)
(*Writes*)


ListPlot[cycles[[{1,3,5}]],PlotLegends->labels[[{1,3,5}]],AxesLabel->{"Bytes","cycles"},PlotRange->All]


(* ::Subsection:: *)
(*Reads*)


ListPlot[cycles[[{2,4}]],PlotLegends->labels[[{2,4}]],AxesLabel->{"Bytes","cycles"},PlotRange->All]


(* ::Subsection:: *)
(*DMA*)


ListPlot[cycles[[{5}]],PlotLegends->labels[[{1,3,5}]],AxesLabel->{"Bytes","cycles"},PlotRange->All]


(* ::Section:: *)
(*Splits per core*)


(* ::Input:: *)
(*fast={"$03:","$07:","$11:","$15:"};*)
(*medium={"$02:","$06:","$10:","$14:"};*)
(*slow={"$01:","$05:","$09:","$13:","$00:","$04:","$08:","$12:"};*)


(* ::Subsection:: *)
(*Busy writes*)


(* ::Input:: *)
(*busywrites=GroupBy[data[[3,2]],First];*)
(*busyspeeds=Map[N[{#[[2]]*8,#[[2]]*8*600/#[[3]]}]&,busywrites,{2}];*)
(*Map[ListPlot[#,PlotRange->{0,25}]&,busyspeeds[[fast]]]*)
(*Map[ListPlot[#,PlotRange->{0,25}]&,busyspeeds[[medium]]]*)
(*Map[ListPlot[#,PlotRange->{0,25}]&,busyspeeds[[slow]]]*)


(* ::Subsection:: *)
(*DMA*)


(* ::Input:: *)
(*dmawrites=GroupBy[data[[5,2]],First];*)
(*Map[ListPlot[#,PlotRange->{0,6000}]&,dmawrites[[fast,All,{2,3}]]]*)
(*Map[ListPlot[#,PlotRange->{0,6000}]&,dmawrites[[medium,All,{2,3}]]]*)
(*Map[ListPlot[#,PlotRange->{0,6000}]&,dmawrites[[slow,All,{2,3}]]]*)


(* ::Section:: *)
(*Zoom on small chunks*)


(* ::Input:: *)
(*smallChunkData=Map[Select[#[[1]]<2500&],speeds[[{1,2}]]];*)
(*plot1=ListPlot[smallChunkData,AxesLabel->{"Bytes","MB/s"},PlotLegends->{"write","read"},PlotLabel->"Core to external memory transfer speeds - nonbusy"]*)


(* ::Input:: *)
(*Export[datapath<>"plot_emem_nodma_nonbusy_zoomed.pdf",plot1]*)


(* ::Input:: *)
(*Export[datapath<>"plot_emem_nodma_nonbusy.pdf",plot1]*)
