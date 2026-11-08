#ifndef BMP_IO_RGB_H
#define BMP_IO_RGB_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#define BMP_GET32(p) ((uint32_t)(p)[0] | (uint32_t)(p)[1]<<8 | (uint32_t)(p)[2]<<16 | (uint32_t)(p)[3]<<24)
#define BMP_PUT32(p,v) do { uint32_t bmp_v_ = (uint32_t)(v); for(int bmp_k_=0; bmp_k_<4; ++bmp_k_) (p)[bmp_k_] = (unsigned char)(bmp_v_ >> (8*bmp_k_)); } while(0)

/* img[N][M][3]: canalele 0=R, 1=G, 2=B; img[0][0] = coltul stanga-sus.
   Fiecare img[i] si img[i][j] este alocat separat. Citire BI_RGB 24/32 bpp.
   Returneaza NULL la eroare; pentru argumente valide, *N si *M raman 0. */
static inline unsigned char ***bmp_read(const char *path, int *N, int *M)
{
    if (!N || !M || N == M) return NULL;
    *N = *M = 0; if (!path) return NULL;
    FILE *f = fopen(path, "rb"); if (!f) return NULL;
    unsigned char h[54], *buf=NULL, ***img=NULL; int nr=0, nc=0;
    if (fread(h, 1, 54, f) != 54 || h[0]!='B' || h[1]!='M') goto fail;
    uint32_t off=BMP_GET32(h+10), dib=BMP_GET32(h+14), w=BMP_GET32(h+18), sh=BMP_GET32(h+22);
    int top = (int)(sh >> 31), b = h[28]/8; uint32_t n = top ? (uint32_t)(0u-sh) : sh;
    if ((dib!=40 && dib!=52 && dib!=56 && dib!=108 && dib!=124) || (uint64_t)off<14ull+dib ||
        h[26]!=1 || h[27] || (h[28]!=24 && h[28]!=32) || h[29] || BMP_GET32(h+30) ||
        !w || w>INT32_MAX || w>INT_MAX || !n || n>INT32_MAX || n>INT_MAX) goto fail;
    uint64_t stride = ((uint64_t)w*b+3)/4*4;
    if (stride>SIZE_MAX || (uint64_t)n*sizeof(*img)>SIZE_MAX || (uint64_t)w*sizeof(**img)>SIZE_MAX) goto fail;
    if (fseek(f, 0, SEEK_END)) goto fail;
    long len = ftell(f);
    if (len<0 || (uint64_t)len<(uint64_t)off+stride*n || fseek(f, (long)off, SEEK_SET)) goto fail;
    img=malloc((size_t)n*sizeof(*img)); buf=malloc((size_t)stride); if (!img || !buf) goto fail;
    for (uint32_t y=0; y<n; ++y) {
        img[y]=malloc((size_t)w*sizeof(**img)); if (!img[y]) goto fail; ++nr; nc=0;
        for (uint32_t x=0; x<w; ++x) { img[y][x]=malloc(3); if (!img[y][x]) goto fail; ++nc; }
    }
    for (uint32_t y=0; y<n; ++y) {
        if (fread(buf, 1, (size_t)stride, f)!=(size_t)stride) goto fail;
        for (size_t x=0; x<w; ++x)
            for (int k=0; k<3; ++k) img[top?y:n-1-y][x][k]=buf[b*x+2-k];
    }
    free(buf); fclose(f); *N=(int)n; *M=(int)w; return img;
fail:
    if (img) { for (int y=0; y<nr; ++y) { for (uint32_t x=0; x<(y==nr-1?(uint32_t)nc:w); ++x) free(img[y][x]); free(img[y]); } free(img); }
    free(buf); fclose(f); return NULL;
}

/* Scrie BMP 24 bpp. Returneaza 1 la succes, 0 la eroare. Nu modifica matricea. */
static inline int bmp_write(const char *path, unsigned char ***img, int N, int M)
{
    if (!path || !img || N<=0 || M<=0) return 0;
    if ((uint64_t)N>INT32_MAX || (uint64_t)M>INT32_MAX) return 0;
    uint64_t stride=((uint64_t)M*3+3)/4*4, bytes=stride*(uint64_t)N;
    if (stride>SIZE_MAX || bytes>UINT32_MAX-54u) return 0;
    for (int y=0; y<N; ++y) { if (!img[y]) return 0; for (int x=0; x<M; ++x) if (!img[y][x]) return 0; }
    unsigned char h[54]={0}, *buf=calloc(1,(size_t)stride); if (!buf) return 0;
    h[0]='B'; h[1]='M'; h[26]=1; h[28]=24;
    BMP_PUT32(h+2,54+bytes); BMP_PUT32(h+10,54); BMP_PUT32(h+14,40);
    BMP_PUT32(h+18,M); BMP_PUT32(h+22,N); BMP_PUT32(h+34,bytes);
    FILE *f=fopen(path,"wb"); if (!f) { free(buf); return 0; }
    int ok=fwrite(h,1,54,f)==54;
    for (int y=N; ok && y-->0;) {
        for (size_t x=0; x<(size_t)M; ++x)
            for (int k=0; k<3; ++k) buf[3*x+2-k]=img[y][x][k];
        ok=fwrite(buf,1,(size_t)stride,f)==(size_t)stride;
    }
    if (fclose(f)!=0) ok=0;
    free(buf); return ok;
}
#undef BMP_GET32
#undef BMP_PUT32
#endif

void dreptunghi(unsigned char ***img, int left, int right, int top, int bottom, int k)
{
    for(int i = bottom; i <= top; i++)
    {
        img[i][left][0] = 255;
        img[i][left][1] = 0;
        img[i][left][2] = 0;

        img[i][right][0] = 255;
        img[i][right][1] = 0;
        img[i][right][2] = 0;

        for(int j = 1; j <= k; j++)
        {
            img[i][left-j][0] = 255;
            img[i][left-j][1] = 0;
            img[i][left-j][2] = 0;

            img[i][right+j][0] = 255;
            img[i][right+j][1] = 0;
            img[i][right+j][2] = 0;
        }
    }

    for(int i = left-k; i <= right+k; i++)
    {
        img[top][i][0] = 255;
        img[top][i][1] = 0;
        img[top][i][2] = 0;

        img[bottom][i][0] = 255;
        img[bottom][i][1] = 0;
        img[bottom][i][2] = 0;

        for(int j = 1; j <= k; j++)
        {
            img[top+j][i][0] = 255;
            img[top+j][i][1] = 0;
            img[top+j][i][2] = 0;

            img[bottom-j][i][0] = 255;
            img[bottom-j][i][1] = 0;
            img[bottom-j][i][2] = 0;
        }
    }
}

void crop(unsigned char ****img_adress, int left, int right, int top, int bottom,int *N_adress, int *M_adress)
{
    int N_vechi = *N_adress, M_vechi = *M_adress;
    *M_adress = right - left + 1;
    *N_adress = top - bottom + 1;

    unsigned char ***img_crop = malloc(*N_adress * sizeof(unsigned char**));

    if(img_crop == NULL)
    {
        printf("aoleu..\n");
        return;
    }

    for(int i = 0; i < *N_adress; i++)
    {
        img_crop[i] = malloc(*M_adress * sizeof(unsigned char*));

        if(img_crop[i] == NULL)
        {
            printf("valeu valeu\n");
            return;
        }

        for(int j = 0; j < *M_adress; j++)
        {
            img_crop[i][j] = malloc(3 * sizeof(unsigned char));
            if(img_crop[i][j] == NULL)
            {
                printf("why..\n");
                return;
            }

            img_crop[i][j][0] = (*img_adress)[bottom+i][left+j][0];
            img_crop[i][j][1] = (*img_adress)[bottom+i][left+j][1];
            img_crop[i][j][2] = (*img_adress)[bottom+i][left+j][2];
        }
    }
    
    for(int i = 0; i < N_vechi; i++)
    {
        for(int j = 0; j < M_vechi; j++)
            free((*img_adress)[i][j]);
        free((*img_adress)[i]);
    }
    free(*img_adress);

    *img_adress = img_crop;
}

void medie_pixeli(unsigned char ***img, int N, int M)
{
    for(int i = 0; i < N; i++)
        for(int j = 0; j < M; j++)
        {
            int medie = (img[i][j][0]+img[i][j][1]+img[i][j][2]) / 3;
            img[i][j][0] = medie;
            img[i][j][1] = medie;
            img[i][j][2] = medie;
        }
}

void completare(unsigned char ****img_adress, int *N_adress, int *M_adress)
{
    if(*N_adress < *M_adress)
    {
        int marime_margine = (*M_adress - *N_adress) / 2;
    
        unsigned char ***img_comp = malloc(*M_adress * sizeof(unsigned char **));

        if(img_comp == NULL)
        {
            printf(";-;");
            return;
        }

        for(int i = 0; i < *M_adress; i++)
        {
            img_comp[i] = malloc(*M_adress * sizeof(unsigned char *));

            if(img_comp[i] == NULL)
            {
                printf("nu mai vreau..");
                return;
            }

            for(int j = 0; j < *M_adress; j++)
            {
                img_comp[i][j] = malloc(3 * sizeof(unsigned char));
                if(img_comp[i][j] == NULL)
                {
                    printf("bruh");
                    return;
                }

                if(i < marime_margine || i >= *N_adress + marime_margine)
                {
                    img_comp[i][j][0] = 255;
                    img_comp[i][j][1] = 255;
                    img_comp[i][j][2] = 255;
                }
                else
                {
                    img_comp[i][j][0] = (*img_adress)[i-marime_margine][j][0];
                    img_comp[i][j][1] = (*img_adress)[i-marime_margine][j][1];
                    img_comp[i][j][2] = (*img_adress)[i-marime_margine][j][2];
                }
            }
        }

        for(int i = 0; i < *N_adress; i++)
        {
            for(int j = 0; j < *M_adress; j++)
                free((*img_adress)[i][j]);
            free((*img_adress)[i]);
        }
        free(*img_adress);

        *N_adress = *M_adress;

        *img_adress = img_comp;
    }
    else
    {
        int marime_margine = (*N_adress - *M_adress) / 2;
        
        unsigned char ***img_comp = malloc(*N_adress * sizeof(unsigned char **));

        if(img_comp == NULL)
        {
            printf(";-;");
            return;
        }

        for(int i = 0; i < *N_adress; i++)
        {
            img_comp[i] = malloc(*N_adress * sizeof(unsigned char *));

            if(img_comp[i] == NULL)
            {
                printf("nu mai vreau..");
                return;
            }

            for(int j = 0; j < *N_adress; j++)
            {
                img_comp[i][j] = malloc(3 * sizeof(unsigned char));
                if(img_comp[i][j] == NULL)
                {
                    printf("bruh");
                    return;
                }

                if(i < marime_margine || i >= *M_adress + marime_margine)
                {
                    img_comp[i][j][0] = 255;
                    img_comp[i][j][1] = 255;
                    img_comp[i][j][2] = 255;
                }
                else
                {
                    img_comp[i][j][0] = (*img_adress)[i][j - marime_margine][0];
                    img_comp[i][j][1] = (*img_adress)[i][j - marime_margine][1];
                    img_comp[i][j][2] = (*img_adress)[i][j - marime_margine][2];
                }
            }
        }

        for(int i = 0; i < *N_adress; i++)
        {
            for(int j = 0; j < *M_adress; j++)
                free((*img_adress)[i][j]);
            free((*img_adress)[i]);
        }
        free(*img_adress);

        *M_adress = *N_adress;

        *img_adress = img_comp;
    }
}

int main(void)
{
    int N, M, left, right, top, bottom, k, caz;

    unsigned char ***img = bmp_read("alpaca.bmp", &N, &M);
    if (!img) return 1;

    printf("Alege cazul: \n1. Dreptunghi\n2. Crop\n3. Medie pixeli idk \n4. Completare \n");

    scanf("%d", &caz);

    switch (caz)
    {
        case 1:
        {
            printf("Introdu indecsii + nr pixeli: ");
            scanf("%d %d %d %d %d", &left, &right, &top, &bottom, &k);

            //Ex: 320 535 500 40 3

            dreptunghi(img, left, right, top, bottom, k);
            break;
        }
        case 2:
        {
            printf("Introdu indecsii: ");
            scanf("%d %d %d %d", &left, &right, &top, &bottom);

            //Ex: 320 535 500 40

            crop(&img, left, right, top, bottom, &N, &M);
            break;
        }
        case 3:
        {
            medie_pixeli(img, N, M);
            break;
        }
        case 4:
        {
            completare(&img, &N, &M);
            break;
        }
    
        default:
        break;
    }

    bmp_write("output.bmp", img, N, M);

    return 0;
}