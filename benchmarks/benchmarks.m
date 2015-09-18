(* ::Package:: *)

(* ::Section:: *)
(*Data import*)


datapath=NotebookDirectory[];
rawdata=Import[datapath<>"benchmarks.txt","Table"];
data=Partition[SplitBy[rawdata,Length],2];
labels=ToString/@Flatten[data[[All,1,All,{2,3,4,5}]],1];
cycles=Map[N[{#[[2]]*8,#[[3]]}]&,data[[All,2]],{2}];
speeds=Map[N[{#[[2]]*8,#[[2]]*8*600/#[[3]]}]&,data[[All,2]],{2}];
cycles=SortBy[First]/@cycles;
speeds=SortBy[First]/@speeds;


(* ::Section:: *)
(*Mean speeds*)


TableForm[Transpose[{Range[0,Length[labels]],{"Type"}~Join~labels,{"Mean"}~Join~Map[Mean,speeds[[All,All,2]]],{"Max"}~Join~Map[Max,speeds[[All,All,2]]]}]]


(* ::Section:: *)
(*Speed plot*)


makeSpeedPlot[datasetIndices_,options_]:=ListPlot[speeds[[datasetIndices]],PlotLegends->labels[[datasetIndices]],AxesLabel->{"Bytes","MB/s"},ImageSize->800,Joined->False,options]


(* ::Subsection:: *)
(*Single-core read/write extmem*)


makeSpeedPlot[{1,2,3,6,7},PlotRange->{{0,2000},{0,300}}]


(* ::Subsection:: *)
(*Busy speeds - write extmem*)


makeSpeedPlot[{4,8},PlotRange->{{0,2000},{0,40}}]


(* ::Subsection:: *)
(*Busy speeds - read extmem*)


makeSpeedPlot[{5,9},PlotRange->{{0,2000},{0,20}}]


(* ::Subsection:: *)
(*Single core - write to core*)


makeSpeedPlot[{10,12,14,16},{}]


(* ::Subsection:: *)
(*Single core - read from core*)


makeSpeedPlot[{11,13,15,17},{}]


(* ::Section:: *)
(*Clockcycles plot*)


makePlot[datasetIndices_,options_]:=ListPlot[cycles[[datasetIndices]],PlotLegends->labels[[datasetIndices]],AxesLabel->{"Bytes","cycles"},ImageSize->800,Joined->True,options]


(* ::Subsection:: *)
(*Single core writes*)


makePlot[{1,2,6},PlotRange->{{0,1000},{0,3000}}]


(* ::Subsection:: *)
(*Writes*)


makePlot[{1,2,4,6,7},{}]


(* ::Subsection:: *)
(*Reads*)


makePlot[{3,5},{}]


(* ::Subsection:: *)
(*DMA*)


makePlot[{6,7},{}]


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
(*Single core burst writes*)


plotCyclesSplitted[1]


(* ::Subsection:: *)
(*Single core non-burst writes*)


plotCyclesSplitted[2]


(* ::Subsection:: *)
(*Busy writes*)


plotSpeedSplitted[4]


(* ::Subsection:: *)
(*DMA*)


plotCyclesSplitted[5]
