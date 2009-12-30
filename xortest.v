module c5 (N1, N2, N4, N5);

input N1, N2;

output N4, N5;

wire N3;

xor XOR2_1 (N3, N1, N2);
and AND2_1 (N4, N1, N3);
not NOT1_1 (N5, N3);

endmodule
