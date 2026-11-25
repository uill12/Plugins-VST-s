from pathlib import Path
import cv2,numpy as np
import sys
src=Path(sys.argv[1]) if len(sys.argv)>1 else Path('art_skin_cp12.png')
img=cv2.imread(str(src),cv2.IMREAD_COLOR)
assert img.shape==(1009,1512,3)
centers=[(1117+(i%4)*106,303) for i in range(4)]+[(1117+(i%4)*106,412) for i in range(4)]+[(1117+(i%4)*106,678) for i in range(4)]+[(1247,867),(1317,867),(1386,867),(1455,867)]
# Remove fixed numeric labels using linear parchment reconstruction from adjacent
# texture at the very same scanlines (no leftover text or blurred inpainting).
for idx,(x,y) in enumerate(centers):
    row=idx//4
    y0,y1=(y+46,y+63) if row<2 else ((y+38,y+56) if row==2 else (y+44,y+58))
    width=40 if row<3 else 33
    x0,x1=x-width,x+width
    for yy in range(y0,y1):
        left=img[yy,x0-14:x0-8].astype('float32').mean(axis=0)
        right=img[yy,x1+8:x1+14].astype('float32').mean(axis=0)
        for xx in range(x0,x1):
            t=(xx-x0)/(x1-x0)
            img[yy,xx]=np.clip(left*(1-t)+right*t,0,255).astype('uint8')
# Replace only the fixed raster dial areas, preserving the typography, parchment,
# approved panel shapes and source art everywhere else. No static needles/arcs.
for idx,(x,y) in enumerate(centers):
    k=1. if idx<12 else .78
    rmax=36 if idx<12 else 32
    iy,ix=np.mgrid[y-rmax:y+rmax+1,x-rmax:x+rmax+1]
    dx=(ix-x).astype('float32');dy=(iy-y).astype('float32');r=np.sqrt(dx*dx+dy*dy)
    angle=(np.degrees(np.arctan2(dy,dx))+360)%360
    ring_sector=(angle>=135)|(angle<=45)
    p=img[y-rmax:y+rmax+1,x-rmax:x+rmax+1].astype('float32')
    rgb=p[...,::-1].copy()
    # Smooth parchment reconstruction in 2D, fitted to 8 corner samples around each dial.
    bkg=np.mean(np.concatenate([rgb[:9,:9].reshape(-1,3),rgb[:9,-9:].reshape(-1,3),rgb[-9:,:9].reshape(-1,3),rgb[-9:,-9:].reshape(-1,3)],axis=0),axis=0)
    shade=dx*.018+dy*.022
    parchment=np.clip(bkg[None,None,:]+shade[...,None],0,255)
    # Ring in relief; the original was an engraved ash-gray horseshoe.
    inner=23*k;ring=28.5*k;outer=32*k
    # A soft outer shadow follows the raised face, including below the missing ring segment.
    shadow=np.clip((31*k-r)/5.0,0,1)*.13
    rgb=rgb*(1-shadow[...,None])+parchment*shadow[...,None]
    band=ring_sector & (r>=25*k) & (r<=32*k)
    gradient=((dx+dy)/(85*k))[...,None]
    grey=np.array([200,192,174],np.float32)[None,None,:]-gradient*6
    aa=np.clip(np.minimum((r-25*k)*1.45,(32*k-r)*1.45),0,1)*band
    rgb=rgb*(1-aa[...,None])+grey*aa[...,None]
    # Ivory machined bezel with warm recess and a gentle down-right face shading.
    bezel=np.clip(np.minimum((r-22*k)*2.2,(25.6*k-r)*2.2),0,1)
    bevel_col=np.array([194,180,150],np.float32)[None,None,:]-((dx+dy)/(85*k))[...,None]*14
    rgb=rgb*(1-bezel[...,None])+bevel_col*bezel[...,None]
    rim=(r>=22*k)&(r<=24*k)
    rim_a=rim.astype('float32')*.83
    rim_rgb=np.array([231,217,191],np.float32)[None,None,:]+((dy-dx)/(85*k))[...,None]*18
    rgb=rgb*(1-rim_a[...,None])+rim_rgb*rim_a[...,None]
    face_a=np.clip((22.8*k-r)*1.4,0,1)
    xsh=np.clip(dx/(24*k),-1,1);ysh=np.clip(dy/(24*k),-1,1)
    face=np.empty_like(rgb)
    face[:,:,0]=232-9*xsh-12*ysh-3*(r/(23*k))**2
    face[:,:,1]=220-11*xsh-14*ysh-4*(r/(23*k))**2
    face[:,:,2]=192-12*xsh-16*ysh-6*(r/(23*k))**2
    rgb=rgb*(1-face_a[...,None])+face*face_a[...,None]
    # Keep an intentionally soft circular boundary, no flat square patch.
    # Erase any remaining fixed needle/arc fragments beyond the new dial rim.
    old_to_new=np.clip((39*k-r)/max(1,3*k),0,1)
    erase=np.clip(old_to_new,0,1) * np.clip(((31 if idx<12 else 28)-dy)/2.0,0,1)
    outer=np.where((r>32*k)[...,None],parchment,rgb)
    rgb=outer*erase[...,None]+p[...,::-1]*(1-erase[...,None])
    img[y-rmax:y+rmax+1,x-rmax:x+rmax+1]=np.clip(rgb[...,::-1],0,255).astype('uint8')
out=Path(sys.argv[2]) if len(sys.argv)>2 else Path('rubedo_art_live_knobs.png')
out.parent.mkdir(parents=True,exist_ok=True)
cv2.imwrite(str(out),img)
img.tofile(out.with_suffix('.bgr'))
from PIL import Image
p=Image.open(out)
p.crop((1050,260,1497,950)).resize((670,1035)).save(out.with_name('preview_clean_knobs.png'))
print('rawbytes',img.nbytes)
