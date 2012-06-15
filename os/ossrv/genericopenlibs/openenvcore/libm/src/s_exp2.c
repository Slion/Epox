/*--------------------------------------------------------------------
 *� Portions copyright (c) 2006 Nokia Corporation.  All rights reserved.
 *--------------------------------------------------------------------
*/
/*-
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef __SYMBIAN32__
__FBSDID("$FreeBSD: src/lib/msun/src/s_exp2.c,v 1.1 2005/04/05 02:57:15 das Exp $");
#endif //__SYMBIAN32__

#include <math.h>
#include "math_private.h"

#define	TBLBITS	8
#define	TBLSIZE	(1 << TBLBITS)

#ifdef __CW32__
static const double
    huge     = 1.07150860718626732094842504906e+301,
    twom1000 = 9.3326361850321887899008954472382e-302,
    redux    = 6755399441055744.0/ TBLSIZE,
    P1	     = 6.9314718246459960e-1,
    P2	     = 2.4022650718688965e-1,
    P3	     = 5.5504109710454940e-2,
    P4	     = 9.6181295812129970e-3,
    P5	     = 1.3333559036254882e-3;


static const double tbl[TBLSIZE * 2] = {
/*	exp2(z + eps)		eps	*/
	0.707106781186592,	9.070522111187529e-14,
0.7090239421602083,	1.332267629550188e-15,
0.7109463010845614,	-4.346523141407488e-14,
0.7128738720527606,	2.731148640577885e-14,
0.7148066691959843,	-1.332267629550188e-15,
0.7167447066838942,	-3.885780586188048e-16,
0.718687998724463,	-5.656586310465173e-14,
0.7206365595642875,	-5.06816810741384e-14,
0.7225904034885321,	1.754152378907747e-14,
0.7245495448209743,	-8.593126210598712e-14,
0.7265139979245225,	-7.438494264988549e-15,
0.7284837772007502,	5.601075159233915e-14,
0.730458897090328,	8.826273045769994e-15,
0.7324393720731814,	-4.229949723821846e-14,
0.7344252166684901,	-1.609823385706477e-15,
0.7364164454346849,	2.109423746787797e-15,
0.738413072969748,	-3.219646771412954e-15,
0.7404151139112507,	2.886579864025407e-14,
0.7424225829363763,	1.110223024625157e-16,
0.7444354947621779,	-4.007905118896815e-14,
0.7464538641456281,	-8.382183835919932e-15,
0.748477705883599,	-3.613775945154885e-14,
0.7505070348132479,	6.750155989720952e-14,
0.7525418658117105,	1.376676550535194e-14,
0.7545822137966949,	-3.153033389935445e-14,
0.756628093726329,	4.585221091701897e-14,
0.7586795205991935,	1.638689184346731e-13,
0.7607365094544245,	3.26405569239796e-14,
0.7627990753722339,	-6.666889262874065e-14,
0.7648672334736256,	-3.375077994860476e-14,
0.766940998920453,	-4.707345624410664e-14,
0.7690203869158267,	-3.219646771412954e-15,
0.7711054127040068,	6.80566714095221e-14,
0.773196091570582,	1.328936960476312e-13,
0.7752924388424706,	-5.473399511402022e-14,
0.7773944698885287,	-2.897682094271659e-14,
0.7795022001189417,	4.29101199017623e-14,
0.781615644985665,	-2.547961841514734e-14,
0.7837348199827712,	-9.71445146547012e-15,
0.7858597406461733,	4.718447854656915e-15,
0.7879904225539545,	2.05391259555654e-14,
0.7901268813264062,	-1.110223024625157e-14,
0.7922691326262677,	3.802513859341161e-14,
0.7944171921585373,	-8.104628079763643e-14,
0.7965710756711293,	-7.549516567451064e-15,
0.7987307989543421,	5.151434834260726e-14,
0.8008963778412981,	-8.754108549169359e-14,
0.8030678282084277,	7.588374373312945e-14,
0.8052451659746497,	4.035660694512444e-14,
0.8074284071024286,	-3.108624468950438e-15,
0.8096175675974437,	2.114974861910923e-14,
0.8118126635086755,	1.976196983832779e-14,
0.8140137109286713,	-4.662936703425657e-15,
0.816220725993644,	1.149080830487037e-14,
0.8184337248835093,	4.768407890765047e-14,
0.8206527238219895,	-2.392530618067212e-14,
0.8228777390769894,	1.215694211964546e-14,
0.8251087869603587,	8.715250743307479e-14,
0.8273458838281059,	1.521005543736464e-14,
0.8295890460808139,	1.010302952408892e-14,
0.8318382901633419,	-4.568567746332519e-14,
0.8340936325653194,	4.873879078104437e-14,
0.8363550898207991,	1.387778780781446e-15,
0.8386226785089406,	2.220446049250313e-15,
0.840896415253685,	-5.073719222536965e-14,
0.8431763167242082,	1.981748098955904e-14,
0.8454623996346857,	5.65381075290361e-14,
0.847754680744676,	1.637578961322106e-14,
0.8500531768593843,	2.080280392391387e-13,
0.8523579048289748,	-8.604228440844963e-14,
0.8546688815502315,	8.326672684688674e-17,
0.8569861239649701,	1.19071419391048e-14,
0.85930964906124,	1.693090112553364e-15,
0.8616394738731314,	-9.214851104388799e-15,
0.863975615480911,	-1.307287611496122e-14,
0.8663180910111634,	1.312838726619248e-14,
0.8686669176368581,	8.271161533457416e-15,
0.8710221125775613,	-2.794986464493832e-14,
0.8733836930995755,	-1.476596622751458e-14,
0.8757516765159298,	-1.532107773982716e-14,
0.8781260801866573,	1.23789867245705e-14,
0.8805069215187851,	-1.121325254871408e-14,
0.882894217966628,	-1.368349877850505e-14,
0.8852879870318328,	9.02333763264096e-14,
0.8876882462632464,	-2.314815006343451e-14,
0.8900950132574994,	-3.449185381754205e-13,
0.892508305659453,	-2.334243909274392e-14,
0.894928141160701,	7.494005416219807e-16,
0.8973545375015584,	7.66053886991358e-15,
0.8997875124702698,	3.58046925441613e-15,
0.9022270839033146,	4.302114220422482e-15,
0.9046732696855097,	-1.001976279724204e-14,
0.9071260877502065,	1.137978600240785e-14,
0.909585556079292,	-1.942890293094024e-14,
0.912051692703543,	2.589595204938178e-14,
0.9145245157024536,	7.827072323607354e-15,
0.9170040432046723,	1.720845688168993e-15,
0.9194902933879413,	-8.715250743307479e-15,
0.9219832844793048,	-1.280919814661274e-14,
0.9244830347552284,	4.6074255521944e-15,
0.9269895625416973,	7.008282842946301e-15,
0.929502886214412,	2.831068712794149e-15,
0.9320230241988915,	-4.690692279041286e-15,
0.9345499949706023,	-2.614575222992244e-14,
0.9370838170551372,	-1.960931417244183e-14,
0.939624509028288,	1.211530875622202e-14,
0.9421720895162452,	1.194044862984356e-13,
0.9447265771954778,	1.261490911730334e-14,
0.9472879907934755,	-1.118549697309845e-14,
0.9498563490882761,	-2.331468351712829e-15,
0.9524316709088301,	-1.064426324859369e-14,
0.955013975135192,	-4.315992008230296e-15,
0.9576032806985753,	2.421673972463623e-15,
0.9601996065815368,	1.963013085415355e-14,
0.9628029718180656,	4.73232564246473e-15,
0.965413395493814,	6.175615574477433e-16,
0.968030896746145,	-3.386180225106727e-15,
0.9706554947643162,	-5.891120924417237e-15,
0.9732872087895824,	-5.073719222536965e-14,
0.9759260581154893,	1.804112415015879e-16,
0.978572062087697,	-4.649058915617843e-15,
0.9812252401044642,	7.355227538141662e-16,
0.9838856116165919,	5.936223734792634e-15,
0.9865531961276164,	-1.131039706336878e-15,
0.9892280131939672,	-1.201122534766341e-14,
0.9919100824251094,	-4.683753385137379e-16,
0.9945994234836228,	-1.498280666201168e-14,
0.9972960560854724,	3.359725692098081e-15,
1,	0,
1.002711275050202,	-2.671474153004283e-16,
1.005429901112792,	-1.606700883449719e-14,
1.008155898118418,	3.642919299551295e-16,
1.010889286051703,	3.507610868425104e-15,
1.013630084951489,	-6.453171330633722e-16,
1.016378314910954,	1.342675970406049e-15,
1.019133996077721,	-2.331121407017633e-14,
1.021897148654111,	-8.222589276130066e-15,
1.024667792897138,	3.858025010572419e-15,
1.027445949118777,	1.877664690397296e-14,
1.030231637686039,	-2.844946500601964e-15,
1.03302487902123,	2.470246229790973e-15,
1.035825693601952,	-7.313594174718219e-15,
1.038634101961379,	-8.326672684688674e-17,
1.041450124688321,	7.022160630754115e-15,
1.04427378242741,	-5.946632075648495e-15,
1.047105095879291,	1.429412144204889e-15,
1.049944085800694,	9.381384558082573e-15,
1.052790773004622,	-5.953570969552402e-15,
1.055645178360575,	2.45775622076394e-14,
1.058507322794506,	-9.284240043427872e-15,
1.061377227289253,	-1.29618538124987e-14,
1.064254912884467,	3.844147222764605e-15,
1.067140400676826,	3.108624468950438e-15,
1.070033711820257,	2.019218126037003e-14,
1.072934867525978,	2.692290834716005e-15,
1.075843889062781,	-1.368349877850505e-14,
1.078760797757122,	2.761679773755077e-15,
1.081685614993217,	3.025357742103552e-15,
1.084618362213316,	9.409140133698202e-15,
1.087559060917776,	8.534839501805891e-15,
1.090507732665291,	4.410360965323434e-14,
1.093464399072879,	-9.686695889854491e-15,
1.096429081816358,	-2.53408405370692e-14,
1.099401802630234,	1.5931700403371e-14,
1.102382583307852,	1.446065489574266e-14,
1.105371445701615,	-1.650068970349139e-13,
1.108368411723707,	3.710920459809586e-14,
1.111373503344809,	-1.160183060733289e-14,
1.114386742595894,	2.164934898019055e-15,
1.117408151567371,	2.969846590872294e-15,
1.120437752409613,	7.799316747991725e-15,
1.123475567333001,	-2.439715096613781e-14,
1.126521618608283,	5.254130464038553e-14,
1.129575928566289,	1.332267629550188e-15,
1.132638519598757,	4.832245714680994e-14,
1.135709414157824,	2.306488333658763e-14,
1.138788634756674,	-2.289834988289385e-14,
1.141876203969568,	8.076872504148014e-15,
1.144972144431791,	-1.712519015484304e-14,
1.148076478840192,	1.637578961322106e-14,
1.151189229952995,	1.582067810090848e-14,
1.154310420590267,	6.369904603786836e-14,
1.157440073633736,	-1.84297022087776e-14,
1.160578212027485,	-1.762479051592436e-14,
1.163724858777586,	1.096345236817342e-14,
1.166880036952455,	-3.2834845953289e-14,
1.170043769683288,	4.654610030740969e-14,
1.173216080163625,	-1.465494392505207e-14,
1.17639699165027,	-1.368349877850505e-14,
1.179586527462872,	-4.468647674116255e-15,
1.182784710984311,	-3.658184866139891e-14,
1.185991565660978,	-1.970645868709653e-14,
1.189207115002668,	-6.4698246760031e-14,
1.192431382582358,	-9.591216709736727e-13,
1.1956643920398,	-3.247402347028583e-14,
1.198906167074349,	-3.835820550079916e-14,
1.202156731452726,	2.736699755701011e-14,
1.205416109005123,	-1.554312234475219e-15,
1.208684323626531,	-5.984102102729594e-14,
1.211961399276829,	3.336220188998595e-14,
1.215247359980493,	2.90878432451791e-14,
1.218542229827392,	-1.98729921407903e-14,
1.221846032972747,	-1.193489751472043e-14,
1.225158793637022,	-1.452726827722017e-13,
1.228480536106879,	1.071365218763276e-14,
1.231811284734168,	1.083577672034153e-13,
1.235151063936936,	3.497202527569243e-15,
1.238499898199799,	-2.098321516541546e-14,
1.241857812073518,	3.941291737419306e-14,
1.245224830175107,	-1.751376821346184e-13,
1.248600977189116,	-1.025290963241332e-13,
1.25198627786635,	3.858025010572419e-14,
1.255380757024715,	2.731148640577885e-14,
1.258784439549701,	-1.720845688168993e-14,
1.262197350394281,	3.480549182199866e-14,
1.265619514578811,	5.773159728050814e-15,
1.269050957191729,	-5.051514762044462e-15,
1.272491703389392,	-1.232347557333924e-14,
1.275941778396378,	-1.643130076445232e-14,
1.279401207505722,	5.995204332975845e-14,
1.282870016078732,	-5.195843755245733e-14,
1.286348229546037,	1.249000902703301e-14,
1.289835873406642,	-2.692290834716005e-14,
1.293332973229099,	1.043609643147647e-14,
1.296839554650994,	-1.731947918415244e-14,
1.300355643379657,	7.327471962526033e-15,
1.303881265191982,	5.051514762044462e-14,
1.307416445934654,	-2.564615186884112e-14,
1.310961211524785,	2.231548279496565e-14,
1.314515587949345,	-1.099120794378905e-14,
1.318079601266059,	-5.773159728050814e-15,
1.321653277603327,	1.849631559025511e-13,
1.325236643159704,	-4.057865155004947e-14,
1.328829724205895,	-6.48925357893404e-14,
1.33243254708332,	1.721955911193618e-13,
1.33604513820418,	3.674838211509268e-14,
1.339667524053417,	1.232902668846236e-13,
1.343299731186792,	-4.596323321948148e-14,
1.346941786232914,	-3.380629109983602e-14,
1.350593715892233,	2.126077092157175e-13,
1.354255546936892,	-5.551115123125783e-16,
1.357927306212955,	5.695444116327053e-14,
1.361609020638187,	-4.046762924758696e-14,
1.365300717204164,	1.605937605120289e-13,
1.369002422974568,	-2.420286193682841e-14,
1.372714165087688,	2.081668171172169e-14,
1.376435970754569,	4.08006961549745e-14,
1.380167867260245,	7.549516567451064e-15,
1.383909881963813,	-1.937339177970898e-14,
1.387662042298513,	-1.670885652060861e-14,
1.391424375771923,	-2.942091015256665e-15,
1.395196909966158,	-4.329869796038111e-14,
1.398979672539303,	1.022737450284694e-12,
1.402772691220125,	-8.215650382226158e-14,
1.406575993819018,	2.498001805406602e-15,
1.410389608217226,	-4.524158825347513e-14
};

#else
static const double
    huge     = 0x1p1000,
    twom1000 = 0x1p-1000,
    redux    = 0x1.8p52 / TBLSIZE,
    P1	     = 0x1.62e42fefa39efp-1,
    P2	     = 0x1.ebfbdff82c575p-3,
    P3	     = 0x1.c6b08d704a0a6p-5,
    P4	     = 0x1.3b2ab88f70400p-7,
    P5	     = 0x1.5d88003875c74p-10;

static const double tbl[TBLSIZE * 2] = {
/*	exp2(z + eps)		eps	*/
	0x1.6a09e667f3d5dp-1,	 0x1.9880p-44,
	0x1.6b052fa751744p-1,	 0x1.8000p-50,
	0x1.6c012750bd9fep-1,	-0x1.8780p-45,
	0x1.6cfdcddd476bfp-1,	 0x1.ec00p-46,
	0x1.6dfb23c651a29p-1,	-0x1.8000p-50,
	0x1.6ef9298593ae3p-1,	-0x1.c000p-52,
	0x1.6ff7df9519386p-1,	-0x1.fd80p-45,
	0x1.70f7466f42da3p-1,	-0x1.c880p-45,
	0x1.71f75e8ec5fc3p-1,	 0x1.3c00p-46,
	0x1.72f8286eacf05p-1,	-0x1.8300p-44,
	0x1.73f9a48a58152p-1,	-0x1.0c00p-47,
	0x1.74fbd35d7ccfcp-1,	 0x1.f880p-45,
	0x1.75feb564267f1p-1,	 0x1.3e00p-47,
	0x1.77024b1ab6d48p-1,	-0x1.7d00p-45,
	0x1.780694fde5d38p-1,	-0x1.d000p-50,
	0x1.790b938ac1d00p-1,	 0x1.3000p-49,
	0x1.7a11473eb0178p-1,	-0x1.d000p-49,
	0x1.7b17b0976d060p-1,	 0x1.0400p-45,
	0x1.7c1ed0130c133p-1,	 0x1.0000p-53,
	0x1.7d26a62ff8636p-1,	-0x1.6900p-45,
	0x1.7e2f336cf4e3bp-1,	-0x1.2e00p-47,
	0x1.7f3878491c3e8p-1,	-0x1.4580p-45,
	0x1.80427543e1b4ep-1,	 0x1.3000p-44,
	0x1.814d2add1071ap-1,	 0x1.f000p-47,
	0x1.82589994ccd7ep-1,	-0x1.1c00p-45,
	0x1.8364c1eb942d0p-1,	 0x1.9d00p-45,
	0x1.8471a4623cab5p-1,	 0x1.7100p-43,
	0x1.857f4179f5bbcp-1,	 0x1.2600p-45,
	0x1.868d99b4491afp-1,	-0x1.2c40p-44,
	0x1.879cad931a395p-1,	-0x1.3000p-45,
	0x1.88ac7d98a65b8p-1,	-0x1.a800p-45,
	0x1.89bd0a4785800p-1,	-0x1.d000p-49,
	0x1.8ace5422aa223p-1,	 0x1.3280p-44,
	0x1.8be05bad619fap-1,	 0x1.2b40p-43,
	0x1.8cf3216b54383p-1,	-0x1.ed00p-45,
	0x1.8e06a5e08664cp-1,	-0x1.0500p-45,
	0x1.8f1ae99157807p-1,	 0x1.8280p-45,
	0x1.902fed0282c0ep-1,	-0x1.cb00p-46,
	0x1.9145b0b91ff96p-1,	-0x1.5e00p-47,
	0x1.925c353aa2ff9p-1,	 0x1.5400p-48,
	0x1.93737b0cdc64ap-1,	 0x1.7200p-46,
	0x1.948b82b5f98aep-1,	-0x1.9000p-47,
	0x1.95a44cbc852cbp-1,	 0x1.5680p-45,
	0x1.96bdd9a766f21p-1,	-0x1.6d00p-44,
	0x1.97d829fde4e2ap-1,	-0x1.1000p-47,
	0x1.98f33e47a23a3p-1,	 0x1.d000p-45,
	0x1.9a0f170ca0604p-1,	-0x1.8a40p-44,
	0x1.9b2bb4d53ff89p-1,	 0x1.55c0p-44,
	0x1.9c49182a3f15bp-1,	 0x1.6b80p-45,
	0x1.9d674194bb8c5p-1,	-0x1.c000p-49,
	0x1.9e86319e3238ep-1,	 0x1.7d00p-46,
	0x1.9fa5e8d07f302p-1,	 0x1.6400p-46,
	0x1.a0c667b5de54dp-1,	-0x1.5000p-48,
	0x1.a1e7aed8eb8f6p-1,	 0x1.9e00p-47,
	0x1.a309bec4a2e27p-1,	 0x1.ad80p-45,
	0x1.a42c980460a5dp-1,	-0x1.af00p-46,
	0x1.a5503b23e259bp-1,	 0x1.b600p-47,
	0x1.a674a8af46213p-1,	 0x1.8880p-44,
	0x1.a799e1330b3a7p-1,	 0x1.1200p-46,
	0x1.a8bfe53c12e8dp-1,	 0x1.6c00p-47,
	0x1.a9e6b5579fcd2p-1,	-0x1.9b80p-45,
	0x1.ab0e521356fb8p-1,	 0x1.b700p-45,
	0x1.ac36bbfd3f381p-1,	 0x1.9000p-50,
	0x1.ad5ff3a3c2780p-1,	 0x1.4000p-49,
	0x1.ae89f995ad2a3p-1,	-0x1.c900p-45,
	0x1.afb4ce622f367p-1,	 0x1.6500p-46,
	0x1.b0e07298db790p-1,	 0x1.fd40p-45,
	0x1.b20ce6c9a89a9p-1,	 0x1.2700p-46,
	0x1.b33a2b84f1a4bp-1,	 0x1.d470p-43,
	0x1.b468415b747e7p-1,	-0x1.8380p-44,
	0x1.b59728de5593ap-1,	 0x1.8000p-54,
	0x1.b6c6e29f1c56ap-1,	 0x1.ad00p-47,
	0x1.b7f76f2fb5e50p-1,	 0x1.e800p-50,
	0x1.b928cf22749b2p-1,	-0x1.4c00p-47,
	0x1.ba5b030a10603p-1,	-0x1.d700p-47,
	0x1.bb8e0b79a6f66p-1,	 0x1.d900p-47,
	0x1.bcc1e904bc1ffp-1,	 0x1.2a00p-47,
	0x1.bdf69c3f3a16fp-1,	-0x1.f780p-46,
	0x1.bf2c25bd71db8p-1,	-0x1.0a00p-46,
	0x1.c06286141b2e9p-1,	-0x1.1400p-46,
	0x1.c199bdd8552e0p-1,	 0x1.be00p-47,
	0x1.c2d1cd9fa64eep-1,	-0x1.9400p-47,
	0x1.c40ab5fffd02fp-1,	-0x1.ed00p-47,
	0x1.c544778fafd15p-1,	 0x1.9660p-44,
	0x1.c67f12e57d0cbp-1,	-0x1.a100p-46,
	0x1.c7ba88988c1b6p-1,	-0x1.8458p-42,
	0x1.c8f6d9406e733p-1,	-0x1.a480p-46,
	0x1.ca3405751c4dfp-1,	 0x1.b000p-51,
	0x1.cb720dcef9094p-1,	 0x1.1400p-47,
	0x1.ccb0f2e6d1689p-1,	 0x1.0200p-48,
	0x1.cdf0b555dc412p-1,	 0x1.3600p-48,
	0x1.cf3155b5bab3bp-1,	-0x1.6900p-47,
	0x1.d072d4a0789bcp-1,	 0x1.9a00p-47,
	0x1.d1b532b08c8fap-1,	-0x1.5e00p-46,
	0x1.d2f87080d8a85p-1,	 0x1.d280p-46,
	0x1.d43c8eacaa203p-1,	 0x1.1a00p-47,
	0x1.d5818dcfba491p-1,	 0x1.f000p-50,
	0x1.d6c76e862e6a1p-1,	-0x1.3a00p-47,
	0x1.d80e316c9834ep-1,	-0x1.cd80p-47,
	0x1.d955d71ff6090p-1,	 0x1.4c00p-48,
	0x1.da9e603db32aep-1,	 0x1.f900p-48,
	0x1.dbe7cd63a8325p-1,	 0x1.9800p-49,
	0x1.dd321f301b445p-1,	-0x1.5200p-48,
	0x1.de7d5641c05bfp-1,	-0x1.d700p-46,
	0x1.dfc97337b9aecp-1,	-0x1.6140p-46,
	0x1.e11676b197d5ep-1,	 0x1.b480p-47,
	0x1.e264614f5a3e7p-1,	 0x1.0ce0p-43,
	0x1.e3b333b16ee5cp-1,	 0x1.c680p-47,
	0x1.e502ee78b3fb4p-1,	-0x1.9300p-47,
	0x1.e653924676d68p-1,	-0x1.5000p-49,
	0x1.e7a51fbc74c44p-1,	-0x1.7f80p-47,
	0x1.e8f7977cdb726p-1,	-0x1.3700p-48,
	0x1.ea4afa2a490e8p-1,	 0x1.5d00p-49,
	0x1.eb9f4867ccae4p-1,	 0x1.61a0p-46,
	0x1.ecf482d8e680dp-1,	 0x1.5500p-48,
	0x1.ee4aaa2188514p-1,	 0x1.6400p-51,
	0x1.efa1bee615a13p-1,	-0x1.e800p-49,
	0x1.f0f9c1cb64106p-1,	-0x1.a880p-48,
	0x1.f252b376bb963p-1,	-0x1.c900p-45,
	0x1.f3ac948dd7275p-1,	 0x1.a000p-53,
	0x1.f50765b6e4524p-1,	-0x1.4f00p-48,
	0x1.f6632798844fdp-1,	 0x1.a800p-51,
	0x1.f7bfdad9cbe38p-1,	 0x1.abc0p-48,
	0x1.f91d802243c82p-1,	-0x1.4600p-50,
	0x1.fa7c1819e908ep-1,	-0x1.b0c0p-47,
	0x1.fbdba3692d511p-1,	-0x1.0e00p-51,
	0x1.fd3c22b8f7194p-1,	-0x1.0de8p-46,
	0x1.fe9d96b2a23eep-1,	 0x1.e430p-49,
	0x1.0000000000000p+0,	 0x0.0000p+0,
	0x1.00b1afa5abcbep+0,	-0x1.3400p-52,
	0x1.0163da9fb3303p+0,	-0x1.2170p-46,
	0x1.02168143b0282p+0,	 0x1.a400p-52,
	0x1.02c9a3e77806cp+0,	 0x1.f980p-49,
	0x1.037d42e11bbcap+0,	-0x1.7400p-51,
	0x1.04315e86e7f89p+0,	 0x1.8300p-50,
	0x1.04e5f72f65467p+0,	-0x1.a3f0p-46,
	0x1.059b0d315855ap+0,	-0x1.2840p-47,
	0x1.0650a0e3c1f95p+0,	 0x1.1600p-48,
	0x1.0706b29ddf71ap+0,	 0x1.5240p-46,
	0x1.07bd42b72a82dp+0,	-0x1.9a00p-49,
	0x1.0874518759bd0p+0,	 0x1.6400p-49,
	0x1.092bdf66607c8p+0,	-0x1.0780p-47,
	0x1.09e3ecac6f383p+0,	-0x1.8000p-54,
	0x1.0a9c79b1f3930p+0,	 0x1.fa00p-48,
	0x1.0b5586cf988fcp+0,	-0x1.ac80p-48,
	0x1.0c0f145e46c8ap+0,	 0x1.9c00p-50,
	0x1.0cc922b724816p+0,	 0x1.5200p-47,
	0x1.0d83b23395dd8p+0,	-0x1.ad00p-48,
	0x1.0e3ec32d3d1f3p+0,	 0x1.bac0p-46,
	0x1.0efa55fdfa9a6p+0,	-0x1.4e80p-47,
	0x1.0fb66affed2f0p+0,	-0x1.d300p-47,
	0x1.1073028d7234bp+0,	 0x1.1500p-48,
	0x1.11301d0125b5bp+0,	 0x1.c000p-49,
	0x1.11edbab5e2af9p+0,	 0x1.6bc0p-46,
	0x1.12abdc06c31d5p+0,	 0x1.8400p-49,
	0x1.136a814f2047dp+0,	-0x1.ed00p-47,
	0x1.1429aaea92de9p+0,	 0x1.8e00p-49,
	0x1.14e95934f3138p+0,	 0x1.b400p-49,
	0x1.15a98c8a58e71p+0,	 0x1.5300p-47,
	0x1.166a45471c3dfp+0,	 0x1.3380p-47,
	0x1.172b83c7d5211p+0,	 0x1.8d40p-45,
	0x1.17ed48695bb9fp+0,	-0x1.5d00p-47,
	0x1.18af9388c8d93p+0,	-0x1.c880p-46,
	0x1.1972658375d66p+0,	 0x1.1f00p-46,
	0x1.1a35beb6fcba7p+0,	 0x1.0480p-46,
	0x1.1af99f81387e3p+0,	-0x1.7390p-43,
	0x1.1bbe084045d54p+0,	 0x1.4e40p-45,
	0x1.1c82f95281c43p+0,	-0x1.a200p-47,
	0x1.1d4873168b9b2p+0,	 0x1.3800p-49,
	0x1.1e0e75eb44031p+0,	 0x1.ac00p-49,
	0x1.1ed5022fcd938p+0,	 0x1.1900p-47,
	0x1.1f9c18438cdf7p+0,	-0x1.b780p-46,
	0x1.2063b88628d8fp+0,	 0x1.d940p-45,
	0x1.212be3578a81ep+0,	 0x1.8000p-50,
	0x1.21f49917ddd41p+0,	 0x1.b340p-45,
	0x1.22bdda2791323p+0,	 0x1.9f80p-46,
	0x1.2387a6e7561e7p+0,	-0x1.9c80p-46,
	0x1.2451ffb821427p+0,	 0x1.2300p-47,
	0x1.251ce4fb2a602p+0,	-0x1.3480p-46,
	0x1.25e85711eceb0p+0,	 0x1.2700p-46,
	0x1.26b4565e27d16p+0,	 0x1.1d00p-46,
	0x1.2780e341de00fp+0,	 0x1.1ee0p-44,
	0x1.284dfe1f5633ep+0,	-0x1.4c00p-46,
	0x1.291ba7591bb30p+0,	-0x1.3d80p-46,
	0x1.29e9df51fdf09p+0,	 0x1.8b00p-47,
	0x1.2ab8a66d10e9bp+0,	-0x1.27c0p-45,
	0x1.2b87fd0dada3ap+0,	 0x1.a340p-45,
	0x1.2c57e39771af9p+0,	-0x1.0800p-46,
	0x1.2d285a6e402d9p+0,	-0x1.ed00p-47,
	0x1.2df961f641579p+0,	-0x1.4200p-48,
	0x1.2ecafa93e2ecfp+0,	-0x1.4980p-45,
	0x1.2f9d24abd8822p+0,	-0x1.6300p-46,
	0x1.306fe0a31b625p+0,	-0x1.2360p-44,
	0x1.31432edeea50bp+0,	-0x1.0df8p-40,
	0x1.32170fc4cd7b8p+0,	-0x1.2480p-45,
	0x1.32eb83ba8e9a2p+0,	-0x1.5980p-45,
	0x1.33c08b2641766p+0,	 0x1.ed00p-46,
	0x1.3496266e3fa27p+0,	-0x1.c000p-50,
	0x1.356c55f929f0fp+0,	-0x1.0d80p-44,
	0x1.36431a2de88b9p+0,	 0x1.2c80p-45,
	0x1.371a7373aaa39p+0,	 0x1.0600p-45,
	0x1.37f26231e74fep+0,	-0x1.6600p-46,
	0x1.38cae6d05d838p+0,	-0x1.ae00p-47,
	0x1.39a401b713ec3p+0,	-0x1.4720p-43,
	0x1.3a7db34e5a020p+0,	 0x1.8200p-47,
	0x1.3b57fbfec6e95p+0,	 0x1.e800p-44,
	0x1.3c32dc313a8f2p+0,	 0x1.f800p-49,
	0x1.3d0e544ede122p+0,	-0x1.7a00p-46,
	0x1.3dea64c1234bbp+0,	 0x1.6300p-45,
	0x1.3ec70df1c4eccp+0,	-0x1.8a60p-43,
	0x1.3fa4504ac7e8cp+0,	-0x1.cdc0p-44,
	0x1.40822c367a0bbp+0,	 0x1.5b80p-45,
	0x1.4160a21f72e95p+0,	 0x1.ec00p-46,
	0x1.423fb27094646p+0,	-0x1.3600p-46,
	0x1.431f5d950a920p+0,	 0x1.3980p-45,
	0x1.43ffa3f84b9ebp+0,	 0x1.a000p-48,
	0x1.44e0860618919p+0,	-0x1.6c00p-48,
	0x1.45c2042a7d201p+0,	-0x1.bc00p-47,
	0x1.46a41ed1d0016p+0,	-0x1.2800p-46,
	0x1.4786d668b3326p+0,	 0x1.0e00p-44,
	0x1.486a2b5c13c00p+0,	-0x1.d400p-45,
	0x1.494e1e192af04p+0,	 0x1.c200p-47,
	0x1.4a32af0d7d372p+0,	-0x1.e500p-46,
	0x1.4b17dea6db801p+0,	 0x1.7800p-47,
	0x1.4bfdad53629e1p+0,	-0x1.3800p-46,
	0x1.4ce41b817c132p+0,	 0x1.0800p-47,
	0x1.4dcb299fddddbp+0,	 0x1.c700p-45,
	0x1.4eb2d81d8ab96p+0,	-0x1.ce00p-46,
	0x1.4f9b2769d2d02p+0,	 0x1.9200p-46,
	0x1.508417f4531c1p+0,	-0x1.8c00p-47,
	0x1.516daa2cf662ap+0,	-0x1.a000p-48,
	0x1.5257de83f51eap+0,	 0x1.a080p-43,
	0x1.5342b569d4edap+0,	-0x1.6d80p-45,
	0x1.542e2f4f6ac1ap+0,	-0x1.2440p-44,
	0x1.551a4ca5d94dbp+0,	 0x1.83c0p-43,
	0x1.56070dde9116bp+0,	 0x1.4b00p-45,
	0x1.56f4736b529dep+0,	 0x1.15a0p-43,
	0x1.57e27dbe2c40ep+0,	-0x1.9e00p-45,
	0x1.58d12d497c76fp+0,	-0x1.3080p-45,
	0x1.59c0827ff0b4cp+0,	 0x1.dec0p-43,
	0x1.5ab07dd485427p+0,	-0x1.4000p-51,
	0x1.5ba11fba87af4p+0,	 0x1.0080p-44,
	0x1.5c9268a59460bp+0,	-0x1.6c80p-45,
	0x1.5d84590998e3fp+0,	 0x1.69a0p-43,
	0x1.5e76f15ad20e1p+0,	-0x1.b400p-46,
	0x1.5f6a320dcebcap+0,	 0x1.7700p-46,
	0x1.605e1b976dcb8p+0,	 0x1.6f80p-45,
	0x1.6152ae6cdf715p+0,	 0x1.1000p-47,
	0x1.6247eb03a5531p+0,	-0x1.5d00p-46,
	0x1.633dd1d1929b5p+0,	-0x1.2d00p-46,
	0x1.6434634ccc313p+0,	-0x1.a800p-49,
	0x1.652b9febc8efap+0,	-0x1.8600p-45,
	0x1.6623882553397p+0,	 0x1.1fe0p-40,
	0x1.671c1c708328ep+0,	-0x1.7200p-44,
	0x1.68155d44ca97ep+0,	 0x1.6800p-49,
	0x1.690f4b19e9471p+0,	-0x1.9780p-45,
};
#endif //__CW32__


/*
 * exp2(x): compute the base 2 exponential of x
 *
 * Accuracy: Peak error < 0.503 ulp for normalized results.
 *
 * Method: (accurate tables)
 *
 *   Reduce x:
 *     x = 2**k + y, for integer k and |y| <= 1/2.
 *     Thus we have exp2(x) = 2**k * exp2(y).
 *
 *   Reduce y:
 *     y = i/TBLSIZE + z - eps[i] for integer i near y * TBLSIZE.
 *     Thus we have exp2(y) = exp2(i/TBLSIZE) * exp2(z - eps[i]),
 *     with |z - eps[i]| <= 2**-9 + 2**-39 for the table used.
 *
 *   We compute exp2(i/TBLSIZE) via table lookup and exp2(z - eps[i]) via
 *   a degree-5 minimax polynomial with maximum error under 1.3 * 2**-61.
 *   The values in exp2t[] and eps[] are chosen such that
 *   exp2t[i] = exp2(i/TBLSIZE + eps[i]), and eps[i] is a small offset such
 *   that exp2t[i] is accurate to 2**-64.
 *
 *   Note that the range of i is +-TBLSIZE/2, so we actually index the tables
 *   by i0 = i + TBLSIZE/2.  For cache efficiency, exp2t[] and eps[] are
 *   virtual tables, interleaved in the real table tbl[].
 *
 *   This method is due to Gal, with many details due to Gal and Bachelis:
 *
 *	Gal, S. and Bachelis, B.  An Accurate Elementary Mathematical Library
 *	for the IEEE Floating Point Standard.  TOMS 17(1), 26-46 (1991).
 */
EXPORT_C double
exp2(double x)
{
	double r, t, z;
	uint32_t hx, hr, ix, lx, i0;
	int k;

	/* Filter out exceptional cases. */
	GET_HIGH_WORD(hx,x);
	ix = hx & 0x7fffffff;		/* high word of |x| */
	if(ix >= 0x40900000) {			/* |x| >= 1024 */
		if(ix >= 0x7ff00000) {
			GET_LOW_WORD(lx,x);
			if(((ix & 0xfffff) | lx) != 0 || (hx & 0x80000000) == 0)
				return (x); 	/* x is NaN or +Inf */
			else 
				return (0.0);	/* x is -Inf */
		}
		#ifdef __SYMBIAN32__
		if(x >= 1024)
		#else
		if(x >= 0x1.0p10)
		#endif //__SYMBIAN32__
			return (huge * huge); /* overflow */
		#ifdef __SYMBIAN32__
		if(x <= -1075)
		#else
		if(x <= -0x1.0ccp10)
		#endif //__SYMBIAN32__
				return (twom1000 * twom1000); /* underflow */
	} else if (ix < 0x3c900000) {		/* |x| < 0x1p-54 */
		return (1.0 + x);
	}

	/* Reduce x, computing z, i0, and k. */
	t = x + redux;
	GET_LOW_WORD(i0, t);
	i0 += TBLSIZE / 2;
	k = (i0 >> TBLBITS) << 20;
	i0 = (i0 & (TBLSIZE - 1)) << 1;
	t -= redux;
	z = x - t;

	/* Compute r = exp2(y) = exp2t[i0] * p(z - eps[i]). */
	t = tbl[i0];		/* exp2t[i0] */
	z -= tbl[i0 + 1];	/* eps[i0]   */
	r = t + t * z * (P1 + z * (P2 + z * (P3 + z * (P4 + z * P5))));

	/* Scale by 2**(k>>20). */
	if(k >= -1021 << 20) {
		if (k != 0) {
			GET_HIGH_WORD(hr, r);
			SET_HIGH_WORD(r, hr + k);
		}
		return (r);
	} else {
		GET_HIGH_WORD(hr, r);
		SET_HIGH_WORD(r, hr + (k + (1000 << 20)));
		return (r * twom1000);
	}
}