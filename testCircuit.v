module kashaypMax7 (a0,b,c,n);

input a,b,c;

output n;

wire g,d,e,f,dh,eh,fh,h,i,j,k,l,m;

nand NAND3_1 (g, a, b, c);

not  NOT1_1 (dh,d);
nand  NAND2_8 (eh,e,e);
nand  NAND2_9 (fh,f,f);

nand NAND2_1 (h, dh, a);
nand NAND2_2 (j, eh, b);
nand NAND2_3 (l, fh, c);

nand NAND2_4 (i, d, g);
nand NAND2_5 (k, e, g);
nand NAND2_6 (m, f, g);

nand NAND6_1 (n , h , i , j , k , l , m);

endmodule

