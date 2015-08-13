(* ::Package:: *)

(* ::Section:: *)
(*Data import*)


datapath=NotebookDirectory[];
rawdata=Import[datapath<>"benchmarks_new.txt","Table"];
data=Partition[SplitBy[rawdata,Length],2];
labels=ToString/@Flatten[data[[All,1,All,{2,3,4,5}]],1];
cycles=Map[N[{#[[2]]*8,#[[3]]}]&,data[[All,2]],{2}];
speeds=Map[N[{#[[2]]*8,#[[2]]*8*600/#[[3]]}]&,data[[All,2]],{2}];


(* ::Section:: *)
(*Mean speeds*)


TableForm[Transpose[{labels,Map[Mean,speeds[[All,All,2]]]}]]


(* ::Section:: *)
(*Speed plot*)


ListPlot[speeds[[All]],PlotLegends->labels[[All]],AxesLabel->{"Bytes","MB/s"},PlotRange->All]


(* ::Section:: *)
(*Clockcycles plot*)


makePlot[datasetIndices_]:=ListPlot[cycles[[datasetIndices]],PlotLegends->labels[[datasetIndices]],AxesLabel->{"Bytes","cycles"},PlotRange->All]


(* ::Subsection:: *)
(*Single core writes*)


makePlot[{1,2,6}]


(* ::Subsection:: *)
(*Writes*)


makePlot[{1,2,4,6,7}]


(* ::Subsection:: *)
(*Reads*)


makePlot[{3,5}]


(* ::Subsection:: *)
(*DMA*)


makePlot[{6,7}]


(* ::Section:: *)
(*Splits per core*)


fast={"$03:","$07:","$11:","$15:"};
medium={"$02:","$06:","$10:","$14:"};
slow={"$01:","$05:","$09:","$13:","$00:","$04:","$08:","$12:"};

plotSpeedSplitted[setIndex_]:=Module[{grouped,speeds},
grouped=GroupBy[data[[setIndex,2]],First];
speeds=Map[N[{#[[2]]*8,#[[2]]*8*600/#[[3]]}]&,grouped,{2}];
Column[{
Map[ListPlot[#,PlotRange->All]&,speeds[[fast]]],
Map[ListPlot[#,PlotRange->All]&,speeds[[medium]]],
Map[ListPlot[#,PlotRange->All]&,speeds[[slow]]]
}]
]

plotCyclesSplitted[setIndex_]:=Module[{grouped},
grouped=GroupBy[data[[setIndex,2]],First];
Column[{
Map[ListPlot[#,PlotRange->All]&,grouped[[fast,All,{2,3}]]],
Map[ListPlot[#,PlotRange->All]&,grouped[[medium,All,{2,3}]]],
Map[ListPlot[#,PlotRange->All]&,grouped[[slow,All,{2,3}]]]
}]
]


(* ::Subsection:: *)
(*Busy writes*)


plotSpeedSplitted[4]


(* ::Subsection:: *)
(*DMA*)


plotCyclesSplitted[5]


(* ::Section:: *)
(*Zoom on small chunks*)


(* ::Input:: *)
(*smallChunkData=Map[Select[#[[1]]<2500&],speeds[[{1,2}]]];*)
(*plot1=ListPlot[smallChunkData,AxesLabel->{"Bytes","MB/s"},PlotLegends->{"write","read"},PlotLabel->"Core to external memory transfer speeds - nonbusy"]*)


(* ::Input:: *)
(*Export[datapath<>"plot_emem_nodma_nonbusy_zoomed.pdf",plot1]*)


(* ::Input:: *)
(*Export[datapath<>"plot_emem_nodma_nonbusy.pdf",plot1]*)
