module c17 (N1,N2,N3,N6,N7,N22,N23);

input N1,N2,N3,N6,N7;

output N22,N23;

wire N10,N11,N16,N19;

nand NAND2_1 (N10, N1, N3);
nand NAND2_2 (N11, N3, N6);
nand NAND2_3 (N16, N2, N11);
nand NAND2_4 (N19, N11, N7);
nand NAND2_5 (N22, N10, N16);
nand NAND2_6 (N23, N16, N19);


endmodule


module TestBenches (N1,N2,N3,N6,N7,N22,N23);

reg N1,N2,N3,N6,N7;

wire N22,N23;


c17 test(N1,N2,N3,N6,N7,N22,N23);

initial
    begin

    N1=0;
    N2=0;
    N3=0;
    N6=0;
    N7=0;

#2
    $display("Outputs are N22:%d N23:%d",N22,N23);
end
