import * as UE from 'ue'
import {rpc, edit_on_instance} from 'ue'

console.warn("Script init of TsTestActor ");

class TsTestActor extends UE.Actor {
    static PrintVersion(){
        console.log("PatchTestJs QuickStart ================== TsTestActor: HotPatcher new 01");
    };
}

export default TsTestActor;
