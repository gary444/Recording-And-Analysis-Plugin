"""
A set of helper functions.

"""
import numpy as np
from os import walk
from os.path import splitext

from scipy.io import loadmat


import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib import use, rcParams
from matplotlib.patches import PathPatch

def get_filenames(path, ftype=None, only_top=0):
    """
    Get list of names of all files in a specified path,
    optionally including subdirectories or filtered by file extention, e.g. '.wav'.
    """

    sdirs = []
    fnames = []
    n_files = []

    for root, dirs, files in walk(path):
        sdirs.extend(dirs)
        if ftype is None:
            fnames.extend(files)
        else:
            for f in files:
                fn, ext = splitext(f)
                if ext == ftype:
                    fnames.extend([f])
        n_files.append(len(files))
    
    return sdirs, fnames, n_files

def splitext_list(str_list):
    """Omit file extention from list of file names and return list of file names without extentions."""
    
    out = []
    
    for fn in str_list:
        f, ext = splitext(fn)
        out.append(f)
    return out

def load_asPatt_all(path, filenames):
    """Load all mat files from list of filename strings."""
    asPatt = []
    for fn in filenames:
        mat = loadmat(path+fn, mat_dtype=True)
        asPatt.append(mat['asPatt'])
    
    return asPatt

def extract_info_from_fname(fname, ftype='apmr_data'):
    """
    Extract dyad_ID, partner_ID and timestamp from filenames of metadata files created in AUD_APMR202202.
    """
    
    # initialize values
    dyad_ID = None
    partner_ID = None
    trial_ID = None
    timestamp = None
    
    fname, _ = splitext(fname)
    
    l = fname.split('_')
    
    if ftype == 'apmr_data':
        dyad_ID = int(l[3])
        partner_ID = int(l[5])
        timestamp = l[-3]+'_'+l[-2]+'_'+l[-1]
        
    elif ftype == 'recordmeta':
        dyad_ID = int(l[3])
        partner_ID = int(l[5])
        trial_ID = int(l[7])
        timestamp = l[-5]+'_'+l[-4]+'_'+l[-3]+'_'+l[-2]+'_'+l[-1]
        
    return dyad_ID, partner_ID, trial_ID, timestamp


    
def get_idx_width_nonzero(a, as_tuple_list = False):
    """Returns indices of first and width until last member of groups of non-zero elements as array or list of tuples."""
    idx_list = np.flatnonzero(np.diff(np.r_[0,a,0])!=0).reshape(-1,2) - [0,1]
    idx_list[:,1] = idx_list[:,1] - idx_list[:,0]
    
    if as_tuple_list:
        idx_list = list(map(tuple,idx_list))
    
    return idx_list

def normalize(x, maximum=1, axis=None, out=None):
    """Normalize a signal to the given maximum (absolute) value.
       Code from https://github.com/spatialaudio/communication-acoustics-exercises/blob/master/tools.py

    Parameters
    ----------
    x : array_like
        Input signal.
    maximum : float or sequence of floats, optional
        Desired (absolute) maximum value.  By default, the signal is
        normalized to +-1.0.  If a sequence is given, it must have the
        same length as the dimension given by `axis`.  Each sub-array
        along the given axis is normalized with one of the values.
    axis : int, optional
        Normalize along a given axis.
        By default, the flattened array is normalized.
    out : numpy.ndarray or similar, optional
        If given, the result is stored in `out` and `out` is returned.
        If `out` points to the same memory as `x`, the normalization
        happens in-place.

    Returns
    -------
    numpy.ndarray
        The normalized signal.

    """
    if axis is None and not np.isscalar(maximum):
        raise TypeError("If axis is not specified, maximum must be a scalar")

    maximum = np.max(np.abs(x), axis=axis) / maximum
    if axis is not None:
        maximum = np.expand_dims(maximum, axis=axis)
    return np.true_divide(x, maximum, out)

# Plotting
def boxplot_all(df, q_list, x='condition', ncols=4, figsize=(20,5), order=None, showmeans=True, sharey=False, rotate_xlabel=False, palette=None, fac=0.8):
    """
    Plot specified number of seaborn boxplots in subplots of figure.
    
    Parameters
    ----------
    df : pandas dataframe
        Dataframe containing the necessary data.
    q_list : list of str
        List of column names in dataframe, e.g. corresponding to questions, rating types, variable names.
    x : str
        Inputs for plotting long-form data. See seaborn.boxplot for further info.
    ncols : int
        Number of columns of the subplot figure.
    figsize : tuple, optional
        Figsize as specified by matplotlib. By default (20,5).  
    order : list of strings
        Order to plot the categorical levels in; otherwise the levels are inferred from the data objects.
    showmeans : bool, optional
        Setting to show means in boxes.
    sharey : bool, optional
        Setting to share y axes of plots in a row.
    rotate_xlabel : bool, optional
        Setting to rotate xlabels.
    palette : palette name, list or dict
        Palette parameter from seaborn.boxplot. By default, None.
    """
    
    nrows = int(np.ceil(len(q_list)/ncols))
    fig, axes = plt.subplots(nrows=nrows, ncols=ncols, figsize=figsize, sharey=sharey)
    if len(q_list)>1:
        axes = axes.flatten()
    if rotate_xlabel:
        for ax in axes:
            ax.tick_params(axis='x', rotation=20)
    
    for i,q in enumerate(q_list):
        if palette is not None:
            _ = sns.boxplot(hue=x, y=q, data=df, width=0.5, order=order, ax=axes[i], showmeans=showmeans, palette=palette,
                            meanprops={"marker": ".", "markeredgecolor": "black","markersize": "10", 'markerfacecolor':'white'})
        else:
            _ = sns.boxplot(x=x, y=q, data=df, width=0.5, order=order, ax=axes[i], showmeans=showmeans,
                            meanprops={"marker": ".", "markeredgecolor": "black","markersize": "10", 'markerfacecolor':'white'})
    adjust_box_widths(fig, fac)
    
    return axes

def adjust_box_widths(g, fac):
    """
    Adjust the withs of a seaborn-generated boxplot.
    """

    # iterating through Axes instances
    for ax in g.axes:

        # iterating through axes artists:
        for c in ax.get_children():

            # searching for PathPatches
            if isinstance(c, PathPatch):
                # getting current width of box:
                p = c.get_path()
                verts = p.vertices
                verts_sub = verts[:-1]
                xmin = np.min(verts_sub[:, 0])
                xmax = np.max(verts_sub[:, 0])
                xmid = 0.5*(xmin+xmax)
                xhalf = 0.5*(xmax - xmin)

                # setting new width of box
                xmin_new = xmid-fac*xhalf
                xmax_new = xmid+fac*xhalf
                verts_sub[verts_sub[:, 0] == xmin, 0] = xmin_new
                verts_sub[verts_sub[:, 0] == xmax, 0] = xmax_new

                # setting new width of median line
                for l in ax.lines:
                    if np.shape(l.get_data())[1]<3 and np.shape(l.get_data())[1]>0:
                        if np.all(l.get_xdata() == [xmin, xmax]):
                            l.set_xdata([xmin_new, xmax_new])

def matplotlib_pgf(EXT):
    """
    Enables pgf export from matplotlib, if EXT='pgf'.
    WARNING: No inline plotting
    """
    if EXT=='pgf':
        use("pgf")
        rcParams.update({
            "pgf.texsystem": "pdflatex",
            'font.family': 'serif',
            'text.usetex': True,
            'pgf.rcfonts': False,
        })
    return EXT