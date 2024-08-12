"""
Python module to generate the trial-condition-mapping as employed in APlausE-MR studies.

Usage: get_tcm(dyad_IDs, 'APMR202202', 'long')
"""

import pandas as pd
import numpy as np

class Condition:
    """
    Class representing condition objects.

    Attributes
    ----------
    var1 : int or bool
        Value or state of corresponding independent variable in corresponding study. Should
        correspond to index of name in var_str_l_tuple[0].
    var2 : int or bool
        Value or state of corresponding independent variable in corresponding study. Should
        correspond to index of name in var_str_l_tuple[1].
    var_str_l_tuple : tuple
        Tuple of list of strings containing names of each variables/factors state,
        e.g. (['DIOTIC', 'SPATIAL'], ['DIST', 'SHAD']) for APMR202202 study representing
        audio and shawowing as independent variables.
    """
    
    def __init__(self, var1, var2, var_str_l_tuple):
        self.var1 = var1
        self.var2 = var2
        
        self.var1_str_l = var_str_l_tuple[0] #tuple of variable string list with var functioning as index
        self.var2_str_l = var_str_l_tuple[1]

    def __str__(self):
        s = self.var1_str_l[self.var1] + '_' + self.var2_str_l[self.var2]
        if s[-1]=='_':
            s = s[:-1]
        
        return s

    def __hash__(self):
        return hash((self.var1, self.var2))

    def __eq__(self, other):
        return (self.var1, self.var1) == (other.var1, other.var2)

    def var_difference(self, second_condition):
        if self.var1 != second_condition.var2:
            return True
        return False

def get_condition(dyad_id, trial_id, possible_orders, as_str=True):
    num_possible_orders = len(possible_orders)
    
    # Return list if no trial_id specified
    if trial_id is not None:
        cond = possible_orders[dyad_id % num_possible_orders][trial_id]
    else:
        cond = possible_orders[dyad_id % num_possible_orders]
        
    # Return as string instead of as Condition object
    if as_str:
        cond = str(cond)
        
    return cond

def get_tcm(group_IDs, study='APMR202202', dform='long'):
    """
    Get trial-condition mapping of specified APlausE-MR study.
    
    Parameters
    ----------
    group_IDs : list or array-like
        List or array of dyad_IDs for which to obtain condition mapping.
    study : str, optional
        APlausE-MR study name/string for corresponding condition structure,
        e.g. \'APMR202202\'.
    dform : str, optional
        Format of returned DataFrame, either \'wide\' or \'long\' (default).

    Returns
    -------
    df : pandas DataFrame object
        DataFrame containing trial-condition mapping in either long or wide format.
    """
    group_ID_name = 'group_ID'
    
    if study=='APMR202201':
        # APMR initial study
        cond_l = ['FACE_TO_FACE', 'SPATIAL', 'NO_SPATIAL']
        var_t = (cond_l, [''])
        possible_orders = [
            [Condition(0, 0, var_t), Condition(1, 0, var_t), Condition(2, 0, var_t)],
            [Condition(2, 0, var_t), Condition(0, 0, var_t), Condition(1, 0, var_t)],
            [Condition(1, 0, var_t), Condition(2, 0, var_t), Condition(0, 0, var_t)],
            [Condition(0, 0, var_t), Condition(2, 0, var_t), Condition(1, 0, var_t)],
            [Condition(1, 0, var_t), Condition(0, 0, var_t), Condition(2, 0, var_t)],
            [Condition(2, 0, var_t), Condition(1, 0, var_t), Condition(0, 0, var_t)]
        ]
        trial_IDs = np.arange(len(possible_orders[0]))
        group_ID_name = 'dyad_ID'
        
    elif study=='APMR202202':
        # APMR second study
        audio_l = ['DIOTIC', 'SPATIAL'] #two audio conditions
        shadowing_l = ['DIST', 'SHAD'] #two different scenes with distributed or shadowing boxes
        var_t =  (audio_l, shadowing_l)
        possible_orders = [
            [Condition(False, False, var_t), Condition(True, False, var_t), Condition(False, True, var_t), Condition(True, True, var_t)],
            [Condition(True, False, var_t), Condition(False, False, var_t), Condition(True, True, var_t), Condition(False, True, var_t)],
            [Condition(False, True, var_t), Condition(True, True, var_t), Condition(False, False, var_t), Condition(True, False, var_t)],
            [Condition(True, True, var_t), Condition(False, True, var_t), Condition(True, False, var_t), Condition(False, False, var_t)]
        ]
        trial_IDs = np.arange(len(possible_orders[0]))
        group_ID_name = 'dyad_ID'

    elif study=='APMR202301':
        # APMR sVR3 triad study
        cond_l = ['SPATIAL', 'DIOTIC', 'REAL']
        scen_l = ['DESERT', 'SEA', 'WINTER']
        var_t =  (cond_l, scen_l)
        possible_orders = [
            [Condition(0, 2, var_t), Condition(1, 0, var_t), Condition(2, 1, var_t)],
            [Condition(0, 2, var_t), Condition(2, 1, var_t), Condition(1, 0, var_t)],
            [Condition(2, 0, var_t), Condition(0, 1, var_t), Condition(1, 2, var_t)],
            [Condition(2, 0, var_t), Condition(1, 2, var_t), Condition(0, 1, var_t)],
            [Condition(1, 1, var_t), Condition(2, 2, var_t), Condition(0, 0, var_t)],
            [Condition(1, 1, var_t), Condition(0, 0, var_t), Condition(2, 2, var_t)]
        ]
        trial_IDs = np.arange(len(possible_orders[0]))

    elif study=='APMR202301_condition':
        # APMR sVR3 triad study actual conditions
        cond_l = ['SPATIAL', 'DIOTIC', 'FACE_TO_FACE']
        var_t = (cond_l, [''])
        possible_orders = [
            [Condition(0, 0, var_t), Condition(1, 0, var_t), Condition(2, 0, var_t)],
            [Condition(0, 0, var_t), Condition(2, 0, var_t), Condition(1, 0, var_t)],
            [Condition(2, 0, var_t), Condition(0, 0, var_t), Condition(1, 0, var_t)],
            [Condition(2, 0, var_t), Condition(1, 0, var_t), Condition(0, 0, var_t)],
            [Condition(1, 0, var_t), Condition(2, 0, var_t), Condition(0, 0, var_t)],
            [Condition(1, 0, var_t), Condition(0, 0, var_t), Condition(2, 0, var_t)]
        ]
        trial_IDs = np.arange(len(possible_orders[0]))

    elif study=='APMR202301_scenario':
        # APMR sVR3 triad study survival scenarios
        cond_l = ['DESERT', 'SEA', 'WINTER']
        var_t = (cond_l, [''])
        possible_orders = [
            [Condition(2, 0, var_t), Condition(0, 0, var_t), Condition(1, 0, var_t)],
            [Condition(2, 0, var_t), Condition(1, 0, var_t), Condition(0, 0, var_t)],
            [Condition(0, 0, var_t), Condition(1, 0, var_t), Condition(2, 0, var_t)],
            [Condition(0, 0, var_t), Condition(2, 0, var_t), Condition(1, 0, var_t)],
            [Condition(1, 0, var_t), Condition(2, 0, var_t), Condition(0, 0, var_t)],
            [Condition(1, 0, var_t), Condition(0, 0, var_t), Condition(2, 0, var_t)]
        ]
        trial_IDs = np.arange(len(possible_orders[0]))
        
    elif study=='APMR202401' or study=='APMR-MR4':
        # APMR MR4 group-to-group projection screen collaborative telepresence study @BUW
        audio_l = ['DIOTIC', 'SPATIAL'] #two audio conditions
        avatar_l = ['ABSTRACT', 'VOLUMETRIC'] #two different user representations based on abstract or volumetric avatars 
        var_t =  (audio_l, avatar_l)
        possible_orders = [
            [Condition(False, True, var_t), Condition(False, False, var_t), Condition(True, False, var_t), Condition(True, True, var_t)],
            [Condition(False, False, var_t), Condition(True, True, var_t), Condition(False, True, var_t), Condition(True, False, var_t)],
            [Condition(True, True, var_t), Condition(True, False, var_t), Condition(False, False, var_t), Condition(False, True, var_t)],
            [Condition(True, False, var_t), Condition(False, True, var_t), Condition(True, True, var_t), Condition(False, False, var_t)]
        ]
        trial_IDs = np.arange(len(possible_orders[0]))
        group_ID_name = 'group_ID'
    
    else:
        raise NameError('Could not find specified study string.')
        
    if dform=='wide':
        keys = ['trial_'+str(t_id) for t_id in trial_IDs]
        tcm = [{group_ID_name:d, **dict(zip(keys, [str(x) for x in get_condition(d, None, possible_orders, False)]))} for d in group_IDs] 
    elif dform=='long':
        tcm = [{group_ID_name:d, 'trial_ID':t, 'condition':get_condition(d, t, possible_orders)} for d in group_IDs for t in trial_IDs]
    else:
        raise NameError('Only \'wide\' and \'long\' format supported.')
    
    df = pd.DataFrame(tcm)    
    
    return df