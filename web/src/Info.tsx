import { Statistic, Row, Col, Button } from 'antd';
import React from 'react';
import { Data } from './format'

interface Props {
    data: Data,
    selectedMapKey: number
}

const Info: React.FC<Props> = (props: Props) => {
    let totalLength = 0;
    let forkCount = 0;
    let leafCount = 0;
    const graph = props.data.graphs[props.selectedMapKey];
    if( graph ){
        for( let i = 0 ; i < graph.sub.length ; i ++ ){
            if( graph.sub[i].arc.length == 1 ) leafCount ++;
            if( graph.sub[i].arc.length > 2 ) forkCount ++;
            for( let j = 0 ; j < graph.sub[i].arc.length ; j ++ ){
                totalLength += graph.sub[i].arc[j].distance;
            }
        }
    }

    return (
        <div className="info">
            {/* <Row gutter={16}>
                <Col span={6}>
                    <Statistic title="总顶点数" value={graph ? graph.sub.length : 0} />
                </Col>
                <Col span={6}>
                    <Statistic title="总距离" value={(totalLength/2).toFixed(2)} precision={2} />
                </Col>
                <Col span={6}>
                    <Statistic title="分支点数" value={forkCount} />
                </Col>
                <Col span={6}>
                    <Statistic title="端点数" value={leafCount} />
                </Col>
            </Row> */}
            <div className="info-block">
                <div className="title">总顶点数</div>
                <div className="number">{graph ? graph.sub.length : 0}</div>
            </div>
            <div className="info-block">
                <div className="title">总距离</div>
                <div className="number">{(totalLength/2).toFixed(2)}</div>
            </div>
            <div className="info-block">
                <div className="title">分支点数</div>
                <div className="number">{forkCount}</div>
            </div>
            <div className="info-block">
                <div className="title">端点数</div>
                <div className="number">{leafCount}</div>
            </div>
        </div>
    )
}

export default Info;