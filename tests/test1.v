module c17 (N0,N1,N2,N3,N6,N7,N23);

input N0,N1,N2,N3,N6,N7;

output N23;

wire N10,N11,N16,N19,N22;

nand NAND2_1 (N10, N1, N0);
nand NAND2_2 (N11, N3, N6);
nand NAND2_3 (N19, N11, N7);
nand NAND2_4 (N22, N10, N2);
nand NAND2_5 (N23, N19, N22);


endmodule


