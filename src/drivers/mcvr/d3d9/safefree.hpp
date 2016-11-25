#ifdef HAVE_XPR_MCVR_DRIVER_D3D

template< typename T_Delete >
inline void SafeDelete( T_Delete **pp )
{
    if(nullptr!=pp && nullptr!=*pp) {
        delete (*pp);
        *pp = nullptr;
    }
}

template< typename T_Array >
inline void SafeDeleteArray( T_Array **pp)
{
    if(nullptr!=pp && nullptr!=*pp) {
        delete [](*pp);
        *pp = nullptr;
    }
}

template< typename T_Release >
inline ULONG SafeRelease( T_Release **pp )
{
    ULONG ret = 0;
    if(nullptr!=pp && nullptr!=*pp) {
        ret = (*pp)->Release();
        *pp = nullptr;
    }
    return ret;
}
#endif // HAVE_XPR_MCVR_DRIVER_D3D